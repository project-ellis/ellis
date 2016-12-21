#include <ellis/codec/json.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/core/err.hpp>
#include <ellis_private/using.hpp>
#include <array>
#include <sstream>

// TODO: support binary blobs


#define ETHROW(CODE, MSG) throw new MAKE_ELLIS_ERR(err_code::CODE, MSG)


namespace ellis {


using std::array;

static char ord(int n)
{
  return (char)(unsigned char)n;
}

static inline int unhex_digit(char ch)
{
  if (ch >= 0 && ch <= 9) {
    return ch - '0';
  }
  else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A';
  }
  else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a';
  }
  else {
    ELLIS_ASSERT_UNREACHABLE();
    // TODO: or, we could return -1, but then we have to make sure the callers
    // all check for this... need a plan that isn't error prone.
  }
}

static const char * k_hexdigits = "0123456789abcdef";

static inline char hex_digit(int i)
{
  ELLIS_ASSERT_GTE(i, 0);
  ELLIS_ASSERT_LTE(i, 15);
  return k_hexdigits[i];
}

static void uni_cp_to_u8(
    int cp,
    std::ostream &os)
{
  /* Single byte UTF-8. */
  if (cp <= 0x7F) {
    os << ord(cp);
    return;
  }

  /* Double byte UTF-8. */
  if (cp <= 0x7FF) {
    os << ord((cp >> 6) + 192)
      << ord((cp & 63) + 128);
    return;
  }

  /* Invalid code points. */
  if (0xd800 <= cp && cp <= 0xdfff) {
    /* Throw it away. */
    return;
  }

  /* Triple byte UTF-8. */
  if (cp <= 0xFFFF) {
    os << ord((cp>>12) + 224)
      << ord(((cp>>6) & 63) + 128)
      << ord((cp & 63) + 128);
    return;
  }

  /* Quadruple byte UTF-8. */
  if (cp <= 0x10FFFF) {
    os << ord((cp>>18) + 240)
      << ord(((cp>>12) & 63) + 128)
      << ord(((cp>>6) & 63) + 128)
      << ord((cp & 63) + 128);
    return;
  }
}


/*  ____  _                              ____  _        _
 * / ___|| |_ _ __ ___  __ _ _ __ ___   / ___|| |_ __ _| |_ _   _ ___
 * \___ \| __| '__/ _ \/ _` | '_ ` _ \  \___ \| __/ _` | __| | | / __|
 *  ___) | |_| | |  __/ (_| | | | | | |  ___) | || (_| | |_| |_| \__ \
 * |____/ \__|_|  \___|\__,_|_| |_| |_| |____/ \__\__,_|\__|\__,_|___/
 *
 */


// TODO: think about whether this really should supplant the
// MUST_CONTINUE and MAY_CONTINUE logic of decoders.

#define ELLIS_STREAM_STATUS_ENTRIES \
  ELLISSS(CONT) \
  ELLISSS(DONE) \
  ELLISSS(ERROR) \
  /* End of ELLIS_STREAM_STATUS_ENTRIES */

enum class stream_status {
#define ELLISSS(X) X,
ELLIS_STREAM_STATUS_ENTRIES
#undef ELLISSS
};

static const char * k_stream_status_names[] = {
#define ELLISSS(X) #X,
ELLIS_STREAM_STATUS_ENTRIES
#undef ELLISSS
};

static inline const char * enum_name(stream_status x)
{
  return k_stream_status_names[(int)x];
}


/*  ____  _
 * / ___|| |_ _ __ ___  __ _ _ __ ___
 * \___ \| __| '__/ _ \/ _` | '_ ` _ \
 *  ___) | |_| | |  __/ (_| | | | | | |
 * |____/ \__|_|  \___|\__,_|_| |_| |_|
 *
 *  ____
 * |  _ \ _ __ ___   __ _ _ __ ___  ___ ___
 * | |_) | '__/ _ \ / _` | '__/ _ \/ __/ __|
 * |  __/| | | (_) | (_| | | |  __/\__ \__ \
 * |_|   |_|  \___/ \__, |_|  \___||___/___/
 *                  |___/
 *
 * This is like stream_status, but it also holds the error details.
 */

class stream_progress {
  stream_status m_status;
  unique_ptr<err> m_err;

public:
  explicit stream_progress(unique_ptr<err> e) :
    m_status(stream_status::ERROR),
    m_err(std::move(e))
  {
  }

  explicit stream_progress(stream_status st) :
    m_status(st),
    m_err()
  {
    ELLIS_ASSERT(st != stream_status::ERROR);
  }

  stream_progress(stream_progress &&o)
  {
    *this = std::move(o);
  }

  stream_progress & operator=(stream_progress &&o)
  {
    if (this != &o) {
      m_err = std::move(o.m_err);
      m_status = o.m_status;
    }
    return *this;
  }

  stream_status stat() const
  {
    return m_status;
  }

  unique_ptr<err> extract_error() {
    return std::move(m_err);
  }
};



/*  _____     _              _                    _        _
 * |_   _|__ | | _____ _ __ (_)_______ _ __   ___| |_ __ _| |_ ___
 *   | |/ _ \| |/ / _ \ '_ \| |_  / _ \ '__| / __| __/ _` | __/ _ \
 *   | | (_) |   <  __/ | | | |/ /  __/ |    \__ \ || (_| | ||  __/
 *   |_|\___/|_|\_\___|_| |_|_/___\___|_|    |___/\__\__,_|\__\___|
 *
 */


#define ELLIS_JSON_TOK_STATE_ENTRIES \
  TOKSTAT(INIT)          /* Ready for new token. */                            \
  TOKSTAT(STRING)        /* Inside a string. */                                \
  TOKSTAT(ESC)           /* Quote escaping mark being applied. */              \
  TOKSTAT(ESC_U1)        /* Expecting 1st unicode codepoint hex digit. */      \
  TOKSTAT(ESC_U2)        /* Expecting 2nd unicode codepoint hex digit. */      \
  TOKSTAT(ESC_U3)        /* Expecting 3rd unicode codepoint hex digit. */      \
  TOKSTAT(ESC_U4)        /* Expecting 4th unicode codepoint hex digit. */      \
  TOKSTAT(NEGSIGN)       /* Just got negative sign, need digit. */             \
  TOKSTAT(ZERO)          /* Have initial zero (can follow with frac or e/E). */\
  TOKSTAT(INT)           /* Have non-zero int (possibly negative). */          \
  TOKSTAT(FRAC)          /* Have int followed by point, require frac digit. */ \
  TOKSTAT(FRACMORE)      /* Accepting possibly more frac digits (or e/E). */   \
  TOKSTAT(EXP)           /* Have e/E, require at least one digit. */           \
  TOKSTAT(EXPSIGN)       /* Have e/E and sign, require at least one digit. */  \
  TOKSTAT(EXPMORE)       /* Have e/E and digit, accept possibly more digits. */\
  TOKSTAT(COMMENTSLASH2) /* Have first slash; waiting for second. */           \
  TOKSTAT(COMMENT)       /* In a // comment; waiting for newline. */           \
  TOKSTAT(BAREWORD)      /* Inside a bare word, might be true/false/null. */   \
  TOKSTAT(END)           /* Parser said done; accept no more until reset. */   \
  TOKSTAT(ERROR)         /* Encountered error; accept no more until reset. */  \
  /* End of ELLIS_JSON_TOK_STATE_ENTRIES */

enum class json_tok_state {
#define TOKSTAT(X) X,
ELLIS_JSON_TOK_STATE_ENTRIES
#undef TOKSTAT
};

static const char * k_json_tok_state_names[] = {
#define TOKSTAT(X) #X,
ELLIS_JSON_TOK_STATE_ENTRIES
#undef TOKSTAT
};

static inline const char * enum_name(json_tok_state x)
{
  return k_json_tok_state_names[(int)x];
}



/*  _____     _                                    _
 * |_   _|__ | | _____ _ __  ___    __ _ _ __   __| |
 *   | |/ _ \| |/ / _ \ '_ \/ __|  / _` | '_ \ / _` |
 *   | | (_) |   <  __/ | | \__ \ | (_| | | | | (_| |
 *   |_|\___/|_|\_\___|_| |_|___/  \__,_|_| |_|\__,_|
 *
 *  _   _                   _                      _             _
 * | \ | | ___  _ __       | |_ ___ _ __ _ __ ___ (_)_ __   __ _| |___
 * |  \| |/ _ \| '_ \ _____| __/ _ \ '__| '_ ` _ \| | '_ \ / _` | / __|
 * | |\  | (_) | | | |_____| ||  __/ |  | | | | | | | | | | (_| | \__ \
 * |_| \_|\___/|_| |_|      \__\___|_|  |_| |_| |_|_|_| |_|\__,_|_|___/
 *
 */


/** This emax function is a templated function meant to return the integer
 * corresponding to the maximum element in a particular simple enum class
 * (one which does not manually specify non-contiguous integral values),
 * by way of specializing the template for the particular class. */

template <typename T>
constexpr int emax();

/* TODO: use emax as part of templatizing the LL parser for other
 * uses besides JSON. */

#define ELLIS_JSON_TOK_ENTRIES \
  JSONTOK(LEFT_CURLY) \
  JSONTOK(RIGHT_CURLY) \
  JSONTOK(COLON) \
  JSONTOK(COMMA) \
  JSONTOK(LEFT_SQUARE) \
  JSONTOK(RIGHT_SQUARE) \
  JSONTOK(STRING) \
  JSONTOK(INTEGER) \
  JSONTOK(REAL) \
  JSONTOK(TRUE) \
  JSONTOK(FALSE) \
  JSONTOK(NIL) \
  JSONTOK(EOS) \
  JSONTOK(ERROR) \
  /* End of ELLIS_JSON_TOK_ENTRIES */

enum class json_tok {
#define JSONTOK(X) X,
ELLIS_JSON_TOK_ENTRIES
#undef JSONTOK
};

static const char * k_json_tok_names[] = {
#define JSONTOK(X) #X,
ELLIS_JSON_TOK_ENTRIES
#undef JSONTOK
};

static inline const char * enum_name(json_tok x)
{
  return k_json_tok_names[(int)x];
}

// TODO: probably can find a way to do this automatically
template <>
constexpr int emax<json_tok>() { return (int)json_tok::ERROR; }

#define ELLIS_JSON_NTS_ENTRIES \
  JSONNTS(VAL) \
  JSONNTS(ARR) \
  JSONNTS(ARR_CONT) \
  JSONNTS(ARR_ETC) \
  JSONNTS(MAP) \
  JSONNTS(MAP_CONT) \
  JSONNTS(MAP_PAIR) \
  JSONNTS(MAP_ETC) \
  JSONNTS(ERROR) \
  /* End of ELLIS_JSON_NTS_ENTRIES */

enum class json_nts {
#define JSONNTS(X) X,
ELLIS_JSON_NTS_ENTRIES
#undef JSONNTS
};

static const char * k_json_nts_names[] = {
#define JSONNTS(X) #X,
ELLIS_JSON_NTS_ENTRIES
#undef JSONNTS
};

static inline const char * enum_name(json_nts x)
{
  return k_json_nts_names[(int)x];
}

template <>
constexpr int emax<json_nts>() { return (int)json_nts::ERROR; }


template <typename TOKTYPE, typename NTSTYPE>
struct tok_nts_union {
  bool m_is_tok;
  union {
    TOKTYPE m_tok;
    NTSTYPE m_nts;
  };
  explicit tok_nts_union(TOKTYPE x) : m_is_tok(true), m_tok(x) {}
  explicit tok_nts_union(NTSTYPE x) : m_is_tok(false), m_nts(x) {}
  bool is_tok() const { return m_is_tok; }
  TOKTYPE tok() const { ELLIS_ASSERT(m_is_tok); return m_tok; }
  NTSTYPE nts() const { ELLIS_ASSERT(!m_is_tok); return m_nts; }
  const char * name() const {
    return m_is_tok ? enum_name(m_tok) : enum_name(m_nts);
  }
};

using json_sym = tok_nts_union<json_tok, json_nts>;



/*  ____                            ____  _        _
 * |  _ \ __ _ _ __ ___  ___ _ __  / ___|| |_ __ _| |_ ___
 * | |_) / _` | '__/ __|/ _ \ '__| \___ \| __/ _` | __/ _ \
 * |  __/ (_| | |  \__ \  __/ |     ___) | || (_| | ||  __/
 * |_|   \__,_|_|  |___/\___|_|    |____/ \__\__,_|\__\___|
 *
 */


struct json_parser_state {
  vector<json_sym> m_syms;
  vector<node> m_nodes;
  string m_key;
  json_tok m_thistok;
  const char * m_thistokstr;
  stream_status m_ststat;

  json_parser_state() {
    reset();
  }

  void reset() {
    m_key.clear();
    m_syms.clear();
    m_nodes.clear();
    m_ststat = stream_status::CONT;
    m_syms.push_back(json_sym(json_nts::VAL));
  }

  void array_swallow() {
    auto n = m_nodes.back();
    m_nodes.pop_back();
    m_nodes.back().as_mutable_array().append(n);
  }

  void map_swallow() {
    auto n = m_nodes.back();
    m_nodes.pop_back();
    m_nodes.back().as_mutable_map().insert(m_key, n);
  }

  unique_ptr<node> extract_node() {
    if (m_nodes.empty()) {
      return nullptr;
    }
    return unique_ptr<node>(new node(m_nodes[0]));
  }
};



/*  ____                            ____        _
 * |  _ \ __ _ _ __ ___  ___ _ __  |  _ \ _   _| | ___  ___
 * | |_) / _` | '__/ __|/ _ \ '__| | |_) | | | | |/ _ \/ __|
 * |  __/ (_| | |  \__ \  __/ |    |  _ <| |_| | |  __/\__ \
 * |_|   \__,_|_|  |___/\___|_|    |_| \_\\__,_|_|\___||___/
 *
 */


/** Production rules, ala BNF, but with rules for building AST.  However, for
 * the moment we wish to constrain that no RHS can result in an "empty" token,
 * to simplify implementation.
 */
struct json_parse_rule {
  json_nts m_lhs;
  const char * m_desc;
  vector<json_sym> m_rhs;
  std::function<void(json_parser_state &state)> m_fn;
};

static const vector<json_parse_rule> g_rules {
  { json_nts::VAL, "VAL --> ARR",
    { json_sym(json_nts::ARR) },
    {} },
  { json_nts::VAL, "VAL --> MAP",
    { json_sym(json_nts::MAP) },
    {} },
  { json_nts::VAL, "VAL --> string",
    { json_sym(json_tok::STRING) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(state.m_thistokstr));
    } },
  { json_nts::VAL, "VAL --> integer",
    { json_sym(json_tok::INTEGER) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(atoi(state.m_thistokstr)));
    } },
  { json_nts::VAL, "VAL --> real",
    { json_sym(json_tok::REAL) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(atof(state.m_thistokstr)));
    } },
  { json_nts::VAL, "VAL --> true",
    { json_sym(json_tok::TRUE) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(true));
    } },
  { json_nts::VAL, "VAL --> false",
    { json_sym(json_tok::FALSE) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(false));
    } },
  { json_nts::VAL, "VAL --> null",
    { json_sym(json_tok::NIL) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(type::NIL));
    } },
  { json_nts::ARR, "ARR --> [ ARR_CONT",
    { json_sym(json_tok::LEFT_SQUARE),
      json_sym(json_nts::ARR_CONT) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(type::ARRAY));
    } },
  { json_nts::ARR_CONT, "ARR_CONT --> ]",
    { json_sym(json_tok::RIGHT_SQUARE) },
    {} },
  { json_nts::ARR_CONT, "ARR_CONT --> VAL ARR_ETC",
    { json_sym(json_nts::VAL),
      json_sym(json_nts::ARR_ETC) },
    {} },
  { json_nts::ARR_ETC, "ARR_ETC --> ]",
    { json_sym(json_tok::RIGHT_SQUARE) },
    [](json_parser_state &state)
    {
      state.array_swallow();
    } },
  { json_nts::ARR_ETC, "ARR_ETC --> , VAL ARR_ETC",
    { json_sym(json_tok::COMMA),
      json_sym(json_nts::VAL),
      json_sym(json_nts::ARR_ETC) },
    [](json_parser_state &state)
    {
      state.array_swallow();
    } },
  { json_nts::MAP, "MAP --> { MAP_CONT",
    { json_sym(json_tok::LEFT_CURLY), json_sym(json_nts::MAP_CONT) },
    [](json_parser_state &state)
    {
      state.m_nodes.push_back(node(type::MAP));
    } },
  { json_nts::MAP_CONT, "MAP_CONT --> }",
    { json_sym(json_tok::RIGHT_CURLY) },
    {} },
  { json_nts::MAP_CONT, "MAP_CONT --> MAP_PAIR MAP_ETC",
    { json_sym(json_nts::MAP_PAIR),
      json_sym(json_nts::MAP_ETC) },
    {} },
  { json_nts::MAP_PAIR, "MAP_PAIR --> string : VAL",
    { json_sym(json_tok::STRING),
      json_sym(json_tok::COLON),
      json_sym(json_nts::VAL) },
    [](json_parser_state &state)
    {
      state.m_key = state.m_thistokstr;
    } },
  { json_nts::MAP_ETC, "MAP_ETC --> }",
    { json_sym(json_tok::RIGHT_CURLY) },
    [](json_parser_state &state)
    {
      state.map_swallow();
    } },
  { json_nts::MAP_ETC, "MAP_ETC --> , MAP_PAIR MAP_ETC",
    { json_sym(json_tok::COMMA),
      json_sym(json_nts::MAP_PAIR),
      json_sym(json_nts::MAP_ETC) },
    [](json_parser_state &state)
    {
      state.map_swallow();
    } }
};



/*  ____
 * |  _ \ __ _ _ __ ___  ___ _ __
 * | |_) / _` | '__/ __|/ _ \ '__|
 * |  __/ (_| | |  \__ \  __/ |
 * |_|   \__,_|_|  |___/\___|_|
 *
 */


class json_parser {
  json_parser_state m_state;
  vector<json_parse_rule> m_rules;
  /* The rule matrix tells you which rule to apply when you have a particular
   * NTS (non-terminating symbol) on the top of the stack, and you are
   * presented with a particular token (i.e. terminating symbol). */
  array<array<int, emax<json_tok>()+1 >, emax<json_nts>()+1 > m_rulematrix;

  /** Initialize the rule matrix based on the rules.
   *
   * This logic has been pulled out of the constructor, both because it is
   * lengthy, and because we may want to do it a single time and re-use the
   * result.
   *
   * Can throw ellis::err if there is a runtime problem detected with rules.
   *
   * If there is any way that a user-provided rule set could have internal
   * inconsistencies, we should throw.  If the inconsistency is with this
   * framework itself, we can assert on that.
   */
  void _initialize_rule_matrix()
  {
    ELLIS_LOG(NOTI, "Initializing json parse rule matrix");
    /* Initialize rule matrix with -1 values (i.e. no rule). */
    for (int ntsidx = 0; ntsidx <= emax<json_nts>(); ntsidx ++) {
      for (int tokidx = 0; tokidx <= emax<json_tok>(); tokidx ++) {
        m_rulematrix[ntsidx][tokidx] = -1;
      }
    }

    /* Make a helper function for the adminstrivial details of updating the
     * rule matrix.  Returns true iff rule matrix was updated. */
    auto update_map = [this](int ntsidx, int tokidx, int ruleidx) {
      ELLIS_ASSERT_GTE(ntsidx, 0);
      ELLIS_ASSERT_LTE(ntsidx, emax<json_nts>());
      ELLIS_ASSERT_GTE(tokidx, 0);
      ELLIS_ASSERT_LTE(tokidx, emax<json_tok>());
      if (m_rulematrix[ntsidx][tokidx] == -1) {
        ELLIS_LOG(INFO, "Updating rule matrix: NTS %d vs token %d uses rule %d",
            ntsidx, tokidx, ruleidx);
        m_rulematrix[ntsidx][tokidx] = ruleidx;
        return true;
      }
      else if (m_rulematrix[ntsidx][tokidx] == ruleidx) {
        return false;
      }
      else {
        /* Uh oh, it's already equal to set to a different rule! */
        ETHROW(TODO, "rule matrix conflict");
      }
    };

    /* Put all directly induced entries into rule matrix (i.e. an NTS maps
     * directly to an RHS starting with the given token). */
    ELLIS_LOG(INFO, "Adding entries for rules direct to token");
    for (int ruleidx = 0; ruleidx < (int)m_rules.size(); ruleidx++) {
      const auto &rule = m_rules[ruleidx];
      ELLIS_ASSERT_GT(rule.m_rhs.size(), 0);
      if (! rule.m_rhs[0].is_tok()) {
        /* Skip rules that don't lead directly to initial tokens. */
        continue;
      }
      if (! update_map((int)rule.m_lhs, (int)(rule.m_rhs[0].tok()), ruleidx)) {
        ETHROW(TODO, "multiple rules map NTS directly to same token");
      }
    }

    /* Update rule matrix with indirectly induced entries (i.e. an NTS maps
     * to another NTS that is known to be capable of starting with a given
     * token) until a cycle through the rule list produces no changes.
     *
     * We know this loop must terminate, because once we record a rule, we
     * don't allow conflicts, and there are a fixed number of cells in the
     * table. */
    ELLIS_LOG(INFO, "Adding entries for rules replacing NTS with NTS");
    while (1) {
      int changes = 0;
      for (int ruleidx = 0; ruleidx < (int)m_rules.size(); ruleidx++) {
        const auto &rule = m_rules[ruleidx];
        if (rule.m_rhs[0].is_tok()) {
          /* Skip rules that lead directly to initial tokens. */
          continue;
        }
        int rhidx = (int)(rule.m_rhs[0].nts());
        /* Loop over all the tokens that the intermediate NTS can reach. */
        for (int tokidx = 0; tokidx <= emax<json_tok>(); tokidx++) {
          if (m_rulematrix[rhidx][tokidx] >= 0) {
            if (update_map((int)rule.m_lhs, tokidx, ruleidx)) {
              changes++;
            }
          }
        }
      }
      ELLIS_LOG(INFO, "Passed through rule list with %d matrix changes",
          changes);
      if (changes == 0) {
        break;
      }
    }
  }

public:
  explicit json_parser(const vector<json_parse_rule> &rules) :
    m_rules(rules)
  {
    ELLIS_LOG(INFO, "Initializing json parser");
    _initialize_rule_matrix();
    reset();
  }

  void reset()
  {
    ELLIS_LOG(INFO, "Resetting json parser");
    m_state.reset();
  }

  stream_progress progdoom(const char *msg)
  {
    ELLIS_LOG(DBUG, "This json parse is doomed--%s", msg);
    /* TODO: improve this message as well as char position etc in caller. */
    ostringstream os;
    os << "parse error, m_syms:";
    for (auto sym : m_state.m_syms) {
      if (sym.is_tok()) {
        /* TODO: string tables for enums. */
        os << " TOK." << (int)(sym.tok());
      }
      else {
        os << " NTS." << (int)(sym.nts());
      }
    }
    os << " " << msg;
    m_state.m_ststat = stream_status::ERROR;
    return stream_progress(unique_ptr<err>(
          new MAKE_ELLIS_ERR(err_code::TODO, os.str())));
  }

  stream_progress progmore()
  {
    ELLIS_LOG(DBUG, "This json parse is continuing...");
    m_state.m_ststat = stream_status::CONT;
    return stream_progress(stream_status::CONT);
  }

  stream_progress progdone()
  {
    ELLIS_LOG(DBUG, "This json parse is done!");
    m_state.m_ststat = stream_status::DONE;
    return stream_progress(stream_status::DONE);
  }

  /** Accept a new token, continue parsing and building the deserialized form.
   *
   * Returns true if document has been successfully built.
   */
  stream_progress accept_token(json_tok tok, const char *tokstr) {
    m_state.m_thistok = tok;
    m_state.m_thistokstr = tokstr;

    ELLIS_LOG(DBUG, "Parser accepting token %s (txt %s)",
        enum_name(tok), tokstr);
    auto & stak = m_state.m_syms;
    if (stak.empty()) {
      return progdoom("parser stack underflow");
    }
    int loopMax = 2500;  // TODO: less arbitrary cycle detection
    /* Apply some rules until we consume the token. */
    for (int i = 0; i < loopMax; i++) {
      ELLIS_LOG(DBUG, "Parse symbol stack: %s",
        std::accumulate(
          m_state.m_syms.begin(),
          m_state.m_syms.end(),
          string(),
          [](string a, json_sym b) { return a + b.name() + ", "; }).c_str());
      auto & top = stak.back();
      if (top.is_tok()) {
        if (top.tok() == tok) {
          ELLIS_LOG(DBUG, "Yay, token %s matches top of stack--popping",
              enum_name(tok));
          stak.pop_back();
          /* If this was the last thing on stack, then done parsing. */
          if (stak.empty()) {
            return progdone();
          } else {
            return progmore();
          }
        } else {
          ELLIS_LOG(DBUG, "Uh oh, token %s does not match top of stack (%s)",
              enum_name(tok), enum_name(top.tok()));
          return progdoom("unexpected token");
        }
      }
      /* Top is a non-terminating symbol; lookup for this token. */
      ELLIS_ASSERT(! top.is_tok());
      ELLIS_LOG(DBUG, "Looking up rule for applying token %s vs NTS %s",
        enum_name(tok), enum_name(top.nts()));
      auto ruleidx = m_rulematrix[(int)(top.nts())][(int)tok];
      if (ruleidx < 0) {
        return progdoom("no applicable production rule for token");
      }
      ELLIS_ASSERT(ruleidx < (int)m_rules.size());
      const auto & rule = m_rules[ruleidx];
      ELLIS_LOG(DBUG, "Applying rule %d (%s)...", ruleidx, rule.m_desc);
      ELLIS_ASSERT(top.nts() == rule.m_lhs);
      stak.pop_back();
      /* Apply the code for this rule, if it is defined. */
      if (rule.m_fn) {
        ELLIS_LOG(DBUG, "Running rule code");
        (rule.m_fn)(m_state);
      }
      /* Push back the translation of the LHS NTS in reverse order. */
      for (auto it = rule.m_rhs.rbegin(); it != rule.m_rhs.rend(); it++) {
        stak.push_back(*it);
      }
    }
    return progdoom("there is some sort of loop in the production rules");
  }

  unique_ptr<node> extract_node()
  {
    ELLIS_LOG(DBUG, "Extracting parsed node");
    ELLIS_ASSERT(m_state.m_ststat == stream_status::DONE);
    ELLIS_ASSERT_EQ(m_state.m_nodes.size(), 1);
    unique_ptr<node> ret(new node(m_state.m_nodes[0]));
    m_state.m_nodes.clear();
    return ret;
  }
};



/*  _____     _              _
 * |_   _|__ | | _____ _ __ (_)_______ _ __
 *   | |/ _ \| |/ / _ \ '_ \| |_  / _ \ '__|
 *   | | (_) |   <  __/ | | | |/ /  __/ |
 *   |_|\___/|_|\_\___|_| |_|_/___\___|_|
 *
 */


class json_tokenizer {
public:
  using tokcb_t = std::function<stream_progress(
      json_tok &tok, const char *str)>;
  json_tok_state m_tokstate;
  std::ostringstream m_txt;
  int m_int;
  int m_digitcount;
  tokcb_t m_tokcb;

  void _clear_txt()
  {
    ELLIS_LOG(DBUG, "Clearing token txt");
    /* Clear the string stream. */
    m_txt.str("");
    /* Reset any stream state such as errors. */
    m_txt.clear();
  }

public:
  json_tokenizer()
  {
    reset();
  }

  void set_token_callback(const tokcb_t &tokcb)
  {
    m_tokcb = tokcb;
  }

  void reset()
  {
    _clear_txt();
    m_tokstate = json_tok_state::INIT;
  }

  stream_progress progdoom(const char *msg)
  {
    ELLIS_LOG(DBUG, "This json tokenizer is doomed--%s", msg);
    m_tokstate = json_tok_state::ERROR;
    return stream_progress(unique_ptr<err>(
          new MAKE_ELLIS_ERR(err_code::TODO, msg)));
  }

  stream_progress progmore()
  {
    ELLIS_LOG(DBUG, "This json tokenizer is continuing");
    return stream_progress(stream_status::CONT);
  }

  stream_progress progdone()
  {
    ELLIS_LOG(DBUG, "This json tokenizer is done");
    m_tokstate = json_tok_state::END;
    return stream_progress(stream_status::DONE);
  }

  stream_progress emit_token(json_tok tok)
  {
    ELLIS_LOG(DBUG, "Emitting token %s to parser", enum_name(tok));
    if (tok == json_tok::ERROR) {
      return progdoom("invalid token");
    }
    stream_progress rv = m_tokcb(tok, m_txt.str().c_str());
    ELLIS_LOG(DBUG, "Post token emission cleanup");
    _clear_txt();
    if (rv.stat() == stream_status::ERROR) {
      m_tokstate = json_tok_state::ERROR;
    }
    else if (rv.stat() == stream_status::DONE) {
      m_tokstate = json_tok_state::END;
    }
    else {
      m_tokstate = json_tok_state::INIT;
    }
    return std::move(rv);
  }

  json_tok token_from_bareword()
  {
    ELLIS_ASSERT(m_tokstate == json_tok_state::BAREWORD);
    auto str = m_txt.str();
    json_tok rv;
    if (str == "true") {
      rv = json_tok::TRUE;
    }
    else if (str == "false") {
      rv = json_tok::FALSE;
    }
    else if (str == "null") {
      rv = json_tok::NIL;
    }
    else {
      rv = json_tok::ERROR;
    }
    ELLIS_LOG(DBUG, "Converted bareword %s to %s token",
        m_txt.str().c_str(), enum_name(rv));
    return rv;
  }

  /** Return the overall stream progress reflecting downstream consumption
   * of tokens pursuant to node construction. */
  stream_progress accept_eos() {
    ELLIS_LOG(DBUG, "Tokenizer received EOS (end of stream)");
    switch (m_tokstate) {
      case json_tok_state::INIT:
        return progmore();

      case json_tok_state::END:
        return progdone();

      case json_tok_state::ERROR:
        return progdoom("error previously encountered");

      case json_tok_state::STRING:
      case json_tok_state::ESC:
      case json_tok_state::ESC_U1:
      case json_tok_state::ESC_U2:
      case json_tok_state::ESC_U3:
      case json_tok_state::ESC_U4:
      case json_tok_state::NEGSIGN:
      case json_tok_state::FRAC:
      case json_tok_state::EXP:
      case json_tok_state::EXPSIGN:
      case json_tok_state::COMMENTSLASH2:
      case json_tok_state::COMMENT:
        return progdoom("stream termination inside of token");

      case json_tok_state::ZERO:
      case json_tok_state::INT:
        return emit_token(json_tok::INTEGER);

      case json_tok_state::FRACMORE:
      case json_tok_state::EXPMORE:
        return emit_token(json_tok::REAL);

      case json_tok_state::BAREWORD:
        {
          return emit_token(token_from_bareword());
        }
    }
    ELLIS_ASSERT_UNREACHABLE();
  }

  stream_progress start_new_token(char ch) {
    ELLIS_LOG(DBUG, "Starting new token based on character (%02X)", (int)ch);
    if (isspace(ch)) {
      /* do nothing--ignore whitespace here. */
    } else if (ch == '/') {
      m_tokstate = json_tok_state::COMMENTSLASH2;
    }
    else if (ch == '{') {
      m_txt << ch;
      return emit_token(json_tok::LEFT_CURLY);
    }
    else if (ch == '}') {
      m_txt << ch;
      return emit_token(json_tok::RIGHT_CURLY);
    }
    else if (ch == '[') {
      m_txt << ch;
      return emit_token(json_tok::LEFT_SQUARE);
    }
    else if (ch == ']') {
      m_txt << ch;
      return emit_token(json_tok::RIGHT_SQUARE);
    }
    else if (ch == ',') {
      m_txt << ch;
      return emit_token(json_tok::COMMA);
    }
    else if (ch == ':') {
      m_txt << ch;
      return emit_token(json_tok::COLON);
    }
    else if (ch == '"') {
      m_tokstate = json_tok_state::STRING;
    }
    else if (ch == '-') {
      m_txt << ch;
      m_tokstate = json_tok_state::NEGSIGN;
    }
    else if (ch == '0') {
      m_txt << ch;
      m_tokstate = json_tok_state::ZERO;
    }
    else if (isdigit(ch)) {
      m_txt << ch;
      m_tokstate = json_tok_state::INT;
    }
    else if (ch == isalpha(ch)) {
      m_txt << ch;
      m_tokstate = json_tok_state::BAREWORD;
    }
    else {
      return progdoom("unexpeced char");
    }
    return progmore();
  }

  /** Emit the given token type, and start on a new token beginning with the
   * given character. */
  stream_progress advance_token(
      json_tok current_tok,
      char nextch)
  {
    auto rv = emit_token(current_tok);
    if (rv.stat() == stream_status::CONT) {
      /* Some patterns (e.g. 0,2) are acceptable but others (e.g. 0"hey")
       * may never occur in the language; still, it is the parser's job to deal
       * with such issues.  We'll start a new token if it looks like we
       * could. */
      return start_new_token(nextch);
    } else {
      return rv;
    }
  }

  stream_progress accept_char(char ch) {
    ELLIS_LOG(DBUG, "Tokenizer accepting character (%02X), current state %s",
        (int)ch, enum_name(m_tokstate));
    switch (m_tokstate) {
      case json_tok_state::INIT:
        return start_new_token(ch);
        break;

      case json_tok_state::STRING:
        if (ch == '\\') {
          m_tokstate = json_tok_state::ESC;
        }
        else if (ch == '"') {
          m_tokstate = json_tok_state::END;
          return emit_token(json_tok::STRING);
        }
        else {
          m_txt << ch;
        }
        break;

      case json_tok_state::ESC:
        if (ch == 'u') {
          m_tokstate = json_tok_state::ESC_U1;
        }
        else if (ch == 'b') {
          m_txt << '\b';
        }
        else if (ch == 'f') {
          m_txt << '\f';
        }
        else if (ch == 'n') {
          m_txt << '\n';
        }
        else if (ch == 'r') {
          m_txt << '\r';
        }
        else if (ch == 't') {
          m_txt << '\t';
        }
        else {
          m_txt << ch;
        }
        break;

      case json_tok_state::ESC_U1:
        if (isxdigit(ch)) {
          m_int = unhex_digit(ch);
          m_tokstate = json_tok_state::ESC_U2;
        }
        else {
          return progdoom("not hex char");
        }
        break;

      case json_tok_state::ESC_U2:
        if (isxdigit(ch)) {
          m_int = m_int * 16 + unhex_digit(ch);
          m_tokstate = json_tok_state::ESC_U3;
        }
        else {
          return progdoom("not hex char");
        }
        break;

      case json_tok_state::ESC_U3:
        if (isxdigit(ch)) {
          m_int = m_int * 16 + unhex_digit(ch);
          m_tokstate = json_tok_state::ESC_U4;
        }
        else {
          return progdoom("not hex char");
        }
        break;

      case json_tok_state::ESC_U4:
        if (isxdigit(ch)) {
          m_int = m_int * 16 + unhex_digit(ch);
          uni_cp_to_u8(m_int, m_txt);
          m_tokstate = json_tok_state::STRING;
        }
        else {
          return progdoom("not hex char");
        }
        break;

      case json_tok_state::NEGSIGN:
        if (ch == '0') {
          m_txt << ch;
          m_tokstate = json_tok_state::ZERO;
        }
        else if (isdigit(ch)) {
          m_txt << ch;
          m_tokstate = json_tok_state::INT;
        }
        else {
          return progdoom("negative sign must be followed by digit");
        }
        break;

      case json_tok_state::ZERO:
        if (ch == '.') {
          m_txt << ch;
          m_tokstate = json_tok_state::FRAC;
        }
        else if (ch == 'e' || ch == 'E') {
          m_txt << ch;
          m_tokstate = json_tok_state::EXP;
        }
        else if (isdigit(ch)) {
          return progdoom("initial zero can not be followed by digits");
        }
        else {
          return advance_token(json_tok::INTEGER, ch);
        }
        break;

      case json_tok_state::INT:
        if (isdigit(ch)) {
          m_txt << ch;
        }
        else if (ch == '.') {
          m_txt << ch;
          m_tokstate = json_tok_state::FRAC;
        }
        else if (ch == 'e' || ch == 'E') {
          m_txt << ch;
          m_tokstate = json_tok_state::EXP;
        }
        else {
          return advance_token(json_tok::INTEGER, ch);
        }
        break;

      case json_tok_state::FRAC:
        if (isdigit(ch)) {
          m_txt << ch;
          m_tokstate = json_tok_state::FRACMORE;
        }
        else {
          return progdoom("decimal point must be followed by digit");
        }
        break;

      case json_tok_state::FRACMORE:
        if (isdigit(ch)) {
          m_txt << ch;
        }
        else if (ch == 'e' || ch == 'E') {
          m_txt << ch;
          m_tokstate = json_tok_state::EXP;
        }
        else {
          return advance_token(json_tok::REAL, ch);
        }
        break;

      case json_tok_state::EXP:
        if (isdigit(ch)) {
          m_txt << ch;
          m_tokstate = json_tok_state::EXPMORE;
        }
        else if (ch == '-' || ch == '+') {
          m_txt << ch;
          m_tokstate = json_tok_state::EXPSIGN;
        }
        else {
          return progdoom("exponent must be followed by sign/digit");
        }
        break;

      case json_tok_state::EXPSIGN:
        if (isdigit(ch)) {
          m_txt << ch;
          m_tokstate = json_tok_state::EXPMORE;
        }
        else {
          return progdoom("exponent sign must be followed by digit");
        }
        break;

      case json_tok_state::EXPMORE:
        if (isdigit(ch)) {
          m_txt << ch;
        }
        else {
          return advance_token(json_tok::REAL, ch);
        }
        break;

      case json_tok_state::COMMENTSLASH2:
        if (ch == '/') {
          m_tokstate = json_tok_state::COMMENT;
        } else {
          return progdoom("bad character after first comment slash");
        }
        break;

      case json_tok_state::COMMENT:
        if (ch == '\n') {
          _clear_txt();
          m_tokstate = json_tok_state::INIT;
        }
        break;

      case json_tok_state::BAREWORD:
        if (isalnum(ch)) {
          m_txt << ch;
        } else {
          return advance_token(token_from_bareword(), ch);
        }
        break;

      case json_tok_state::END:
        return progdone();
        break;

      case json_tok_state::ERROR:
        return progdoom("prior error");
    }
    /* If we didn't return earlier, we are in the middle of a token and can
     * accept more input. */
    return progmore();
  }
};


/*  ____                     _
 * |  _ \  ___  ___ ___   __| | ___ _ __
 * | | | |/ _ \/ __/ _ \ / _` |/ _ \ '__|
 * | |_| |  __/ (_| (_) | (_| |  __/ |
 * |____/ \___|\___\___/ \__,_|\___|_|
 *
 */


json_decoder::json_decoder() :
  m_toker(new json_tokenizer()),
  m_parser(new json_parser(g_rules))
{
  m_toker->set_token_callback(
      [this](json_tok tok, const char *tokstr)
      {
        return m_parser->accept_token(tok, tokstr);
      });
}

json_decoder::~json_decoder()
{
}

//void json_decoder::_take_token(json_tok tok)
//{
//  /* Extract appropriate node. */
//  unique_ptr<node> new_node;
//  if (tok == json_tok::STRING) {
//    new_node.reset(new node(m_txt.str()));
//  }
//  else if (tok == json_tok::INTEGER) {
//    auto str = m_txt.str();
//    char *endptr;
//    new_node.reset(new node(strtol(str.c_str(), &endptr, 10)));
//    if (endptr == str.c_str()) {
//      // TODO: error
//    }
//  }
//  else if (tok == json_tok::REAL) {
//    auto str = m_txt.str();
//    char *endptr;
//    new_node.reset(new node(strtod(str.c_str(), &endptr)));
//    if (endptr == str.c_str()) {
//      // TODO: error
//    }
//  }
//  else if (tok == json_tok::BAREWORD) {
//    auto str == m_txt.str();
//    if (str == "true") {
//      new_node.reset(new node(true));
//    }
//    else if (str == "false") {
//      new_node.reset(new node(false));
//    }
//    else if (str == "null") {
//      new_node.reset(new node(type::NIL));
//    }
//    else {
//      // TODO: error
//    }
//  }
//
//}

decoding_status json_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  ELLIS_LOG(DBUG, "Consuming buffer of length %zu", *bytecount);
  const byte *p_end = buf + *bytecount;
  for (const byte *p = buf; p < p_end; p++) {
    auto st = m_toker->accept_char(*p);
    ELLIS_LOG(DBUG, "Tokenizer state: %s", enum_name(st.stat()));
    if (st.stat() == stream_status::DONE) {
      *bytecount = p_end - p;
      return decoding_status::END;
    }
    else if (st.stat() == stream_status::ERROR) {
      *bytecount = p_end - p;
      m_err = st.extract_error();
      return decoding_status::ERROR;
    }
  }
  *bytecount = 0;
  return decoding_status::MUST_CONTINUE;
}

unique_ptr<node> json_decoder::extract_node()
{
  ELLIS_LOG(DBUG, "Extracting node from decoder (will send EOS to tokenizer)");
  /* Send EOS to tokenizer. */
  auto st = m_toker->accept_eos();

  if (st.stat() == stream_status::ERROR) {
    if (! m_err) {
      m_err = st.extract_error();
    }
    return nullptr;
  }
  else if (st.stat() == stream_status::CONT) {
    if (! m_err) {
      m_err.reset(new MAKE_ELLIS_ERR(err_code::TODO, "no node"));
    }
    return nullptr;
  }

  return m_parser->extract_node();
}

unique_ptr<err> json_decoder::extract_error()
{
  ELLIS_LOG(DBUG, "Extracting error from decoder");
  return std::move(m_err);
}

void json_decoder::reset()
{
  ELLIS_LOG(DBUG, "Resetting decoder");
  m_toker->reset();
  m_parser->reset();
}


/*  _____                     _
 * | ____|_ __   ___ ___   __| | ___ _ __
 * |  _| | '_ \ / __/ _ \ / _` |/ _ \ '__|
 * | |___| | | | (_| (_) | (_| |  __/ |
 * |_____|_| |_|\___\___/ \__,_|\___|_|
 *
 */


void json_encoder::_clear_obuf() {
  /* Clear the string stream. */
  m_obuf.str("");
  /* Reset any stream state such as errors. */
  m_obuf.clear();
}

json_encoder::json_encoder()
{
}

encoding_status json_encoder::fill_buffer(
    byte *buf,
    size_t *bytecount)
{
  size_t ss_avail = m_obufend - m_obufpos;
  size_t actual_bc = std::min(*bytecount, ss_avail);
  m_obuf.read((char *)buf, actual_bc);
  m_obufpos += actual_bc;
  *bytecount = actual_bc;
  if (m_obufpos == m_obufend) {
    return encoding_status::END;
  }
  return encoding_status::CONTINUE;
}

unique_ptr<err> json_encoder::extract_error()
{
  return std::move(m_err);
}

void json_encoder::stream_out(const node &n, std::ostream &os) {
  switch (n.get_type()) {
    case type::NIL:
      os << "null";
      return;

    case type::BOOL:
      os << ((n.as_bool()) ? "true" : "false");
      return;

    case type::INT64:
      os << (n.as_int64());
      return;

    case type::DOUBLE:
      os << (n.as_double());
      return;

    case type::U8STR:
      os << '"' << (n.as_u8str()) << '"';
      return;

    case type::ARRAY:
      {
        os << "[ ";
        const auto &a = n.as_array();
        bool separate = false;
        for (size_t i = 0; i < a.length(); i++) {
          if (separate) {
            os << ", ";
          }
          separate = true;
          stream_out(a[i], os);
        }
        os << " ]";
      }
      return;

    case type::BINARY:
      {
        // TODO: mime encode, not this
        os << "\"/ELLIS_BINARY/";
        const auto &a = n.as_binary();
        for (size_t i = 0; i < a.length(); i++) {
          byte b = a[i];
          os << "x" << hex_digit(b >> 8) << hex_digit(b & 15);
          stream_out(a[i], os);
        }
        os << "]";
      }
      return;

    case type::MAP:
      {
        os << "{ ";
        const auto &a = n.as_map();
        bool separate = false;
        auto keys = a.keys();
        for (const auto &k : keys) {
          if (separate) {
            os << ", ";
          }
          separate = true;
          os << '"' << k << '"' << ": ";
          stream_out(a[k], os);
        }
        os << " }";
      }
      return;
  }
}

void json_encoder::reset(const node *new_node)
{
  _clear_obuf();

  stream_out(*new_node, m_obuf);
  m_obuf.flush();
  m_obufpos = 0;
  m_obufend = m_obuf.tellp();
}


}  /* namespace ellis */
