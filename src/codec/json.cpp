#include <ellis/codec/json.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/disposition.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/u8str_node.hpp>
#include <ellis_private/using.hpp>
#include <array>
#include <numeric>

// TODO: support binary blobs


namespace ellis {


using std::array;

static char ord(int n)
{
  return (char)(unsigned char)n;
}

static inline int unhex_digit(byte b)
{
  if (b >= '0' && b <= '9') {
    return b - '0';
  }
  else if (b >= 'A' && b <= 'F') {
    return b - 'A' + 10;
  }
  else if (b >= 'a' && b <= 'f') {
    return b - 'a' + 10;
  }
  else {
    THROW_ELLIS_ERR(INVALID_ARGS,
        "not a hexadecimal char (dec " << (unsigned int)b << ")");
  }
}

static const char * k_hexdigits = "0123456789abcdef";

static inline char hex_digit(int i)
{
  if (i < 0 || i > 15) {
    THROW_ELLIS_ERR(INVALID_ARGS,
        "digit (dec " << i << ") out of hexadecimal range");
  }
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
  enum_max = ERROR
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
  enum_max = ERROR
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
  enum_max = ERROR
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

  json_parser_state() {
    reset();
  }

  void reset() {
    m_key.clear();
    m_syms.clear();
    m_nodes.clear();
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
      state.m_nodes.push_back(node(atol(state.m_thistokstr)));
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
  array<array<int, (int)json_tok::enum_max+1 >, (int)json_nts::enum_max+1 > m_rulematrix;

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
    for (int ntsidx = 0; ntsidx <= (int)json_nts::enum_max; ntsidx ++) {
      for (int tokidx = 0; tokidx <= (int)json_tok::enum_max; tokidx ++) {
        m_rulematrix[ntsidx][tokidx] = -1;
      }
    }

    /* Make a helper function for the adminstrivial details of updating the
     * rule matrix.  Returns true iff rule matrix was updated. */
    auto update_map = [this](int ntsidx, int tokidx, int ruleidx) {
      ELLIS_ASSERT_GTE(ntsidx, 0);
      ELLIS_ASSERT_LTE(ntsidx, (int)json_nts::enum_max);
      ELLIS_ASSERT_GTE(tokidx, 0);
      ELLIS_ASSERT_LTE(tokidx, (int)json_tok::enum_max);
      if (m_rulematrix[ntsidx][tokidx] == -1) {
        ELLIS_LOG(INFO, "Updating rule matrix: %s vs %s uses rule %d (%s)",
            enum_name(json_nts(ntsidx)), enum_name(json_tok(tokidx)),
            ruleidx, m_rules[ruleidx].m_desc);
        m_rulematrix[ntsidx][tokidx] = ruleidx;
        return true;
      }
      else if (m_rulematrix[ntsidx][tokidx] == ruleidx) {
        return false;
      }
      else {
        /* Uh oh, it's already equal to set to a different rule! */
        THROW_ELLIS_ERR(INTERNAL,
            "rule conflict: rule matrix for NTS ("
            << enum_name(json_nts(ntsidx))
            << ") vs token ("
            << enum_name(json_tok(tokidx))
            << ") covered by two different rules ("
            << ruleidx <<
            " and "
            << m_rulematrix[ntsidx][tokidx]
            << ")");
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
      update_map((int)rule.m_lhs, (int)(rule.m_rhs[0].tok()), ruleidx);
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
        for (int tokidx = 0; tokidx <= (int)json_tok::enum_max; tokidx++) {
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

  node_progress progdoom(const char *msg)
  {
    ELLIS_LOG(DBUG, "This json parse is doomed--%s", msg);
    /* TODO: improve this message as well as char position etc in caller. */
    ostringstream os;
    os << "parse error, parsing symbol stack was:";
    for (auto sym : m_state.m_syms) {
      os << " " << sym.name();
    }
    os << " " << msg;
    return node_progress(MAKE_UNIQUE_ELLIS_ERR(PARSE_FAIL, os.str()));
  }

  node_progress progmore()
  {
    ELLIS_LOG(DBUG, "This json parse is continuing...");
    return node_progress(stream_state::CONTINUE);
  }

  node_progress progdone()
  {
    ELLIS_LOG(DBUG, "This json parse is done!");
    ELLIS_ASSERT_EQ(m_state.m_nodes.size(), 1);
    unique_ptr<node> ret(make_unique<node>(m_state.m_nodes[0]));
    m_state.m_nodes.clear();
    return node_progress(std::move(ret));
  }

  /** Accept a new token, continue parsing and building the deserialized form.
   *
   * Returns true if document has been successfully built.
   */
  node_progress accept_token(json_tok tok, const char *tokstr) {
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
  using tokcb_t = std::function<node_progress(
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

  node_progress progdoom(const char *msg)
  {
    ELLIS_LOG(DBUG, "This json tokenizer is doomed--%s", msg);
    m_tokstate = json_tok_state::ERROR;
    return node_progress(MAKE_UNIQUE_ELLIS_ERR(PARSE_FAIL, msg));
  }

  node_progress progmore()
  {
    ELLIS_LOG(DBUG, "This json tokenizer is continuing");
    return node_progress(stream_state::CONTINUE);
  }

  node_progress emit_token(json_tok tok)
  {
    ELLIS_LOG(DBUG, "Emitting token %s to parser", enum_name(tok));
    if (tok == json_tok::ERROR) {
      return progdoom("invalid token");
    }
    node_progress rv = m_tokcb(tok, m_txt.str().c_str());
    ELLIS_LOG(DBUG, "Post token emission cleanup");
    _clear_txt();
    if (rv.state() == stream_state::ERROR) {
      m_tokstate = json_tok_state::ERROR;
    }
    else if (rv.state() == stream_state::SUCCESS) {
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

  /**
   * Terminate the reconstruction here--no more bytes for this datum.
   *
   * Returns the resulting state of node reconstruction (the overall stream
   * disposition reflecting downstream consumption of tokens pursuant to node
   * reconstruction).
   */
  node_progress chop() {
    ELLIS_LOG(DBUG, "Tokenizer received EOS (end of stream)");
    switch (m_tokstate) {
      case json_tok_state::INIT:
        return progmore();

      case json_tok_state::END:
        return progdoom("already returned result");

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

  /**
   * Start a new token based on the given char, as if from INIT state.
   *
   * If this char constitutes a complete token, it will be supplied to the
   * parser, and the status ferried back.
   *
   * Returns the resulting state of node reconstruction.
   */
  node_progress start_new_token(char ch) {
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
    else if (isalpha(ch)) {
      m_txt << ch;
      m_tokstate = json_tok_state::BAREWORD;
    }
    else {
      return progdoom("unexpeced char");
    }
    return progmore();
  }

  /**
   * Emit the given token type, and start on a new token beginning with the
   * given character.
   *
   * Returns the resulting state of node reconstruction.
   */
  node_progress advance_token(
      json_tok current_tok,
      char nextch)
  {
    auto rv = emit_token(current_tok);
    if (rv.state() == stream_state::CONTINUE) {
      /* Some patterns (e.g. 0,2) are acceptable but others (e.g. 0"hey")
       * may never occur in the language; still, it is the parser's job to deal
       * with such issues.  We'll start a new token if it looks like we
       * could. */
      return start_new_token(nextch);
    } else {
      return rv;
    }
  }

  /**
   * Advance the tokenizing process by one character, emitting tokens to the
   * parser as needed.
   *
   * Returns the resulting state of node reconstruction.
   */
  node_progress accept_char(char ch) {
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
          m_tokstate = json_tok_state::INIT;
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
        else {
          if (ch == 'b') {
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
          m_tokstate = json_tok_state::STRING;
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
        return progdoom("already returned result");
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
  m_toker(make_unique<json_tokenizer>()),
  m_parser(make_unique<json_parser>(g_rules))
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

node_progress json_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  ELLIS_LOG(DBUG, "Consuming buffer of length %zu", *bytecount);
  const byte *p_end = buf + *bytecount;
  for (const byte *p = buf; p < p_end; p++) {
    auto st = m_toker->accept_char(*p);
    ELLIS_LOG(DBUG, "Tokenizer state: %s", enum_name(st.state()));
    if (st.state() == stream_state::SUCCESS
        || st.state() == stream_state::ERROR) {
      *bytecount = p_end - p;
      return st;
    }
  }
  *bytecount = 0;
  return node_progress(stream_state::CONTINUE);
}

node_progress json_decoder::chop()
{
  /* Send EOS to tokenizer. */
  auto st = m_toker->chop();

  ELLIS_LOG(DBUG, "Tokenizer state: %s", enum_name(st.state()));
  if (st.state() == stream_state::CONTINUE) {
    return node_progress(
          MAKE_UNIQUE_ELLIS_ERR(PARSE_FAIL, "truncated input"));
  }
  return st;
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

void json_encoder::_stream_out(const node &n, std::ostream &os) {
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
      os << std::to_string(n.as_double());
      return;

    case type::U8STR:
      {
        os << '"';
        // TODO: iterators over u8str_node
        const string s(n.as_u8str().c_str());
        for (auto &ch : s) {
          if (ch == '"' || ch == '\\' || ch == '/') {
            os << '\\' << ch;
          }
          else if (ch == '\b') {
            os << '\\' << 'b';
          }
          else if (ch == '\f') {
            os << '\\' << 'f';
          }
          else if (ch == '\n') {
            os << '\\' << 'n';
          }
          else if (ch == '\r') {
            os << '\\' << 'r';
          }
          else if (ch == '\t') {
            os << '\\' << 't';
          }
          else if ((unsigned)ch <= 0x1f) {
            unsigned char u = (unsigned char)ch;
            os << "\\u00" << hex_digit(u >> 4) << hex_digit(u & 15);
          }
          else {
            os << ch;
          }
        }
        os << '"';
      }
      return;

    case type::ARRAY:
      {
        os << "[";
        const auto &a = n.as_array();
        bool separate = false;
        for (size_t i = 0; i < a.length(); i++) {
          if (separate) {
            os << ", ";
          }
          else {
            os << " ";
          }
          separate = true;
          _stream_out(a[i], os);
        }
        if (separate) {
          os << " ";
        }
        os << "]";
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
          _stream_out(a[i], os);
        }
        os << "]";
      }
      return;

    case type::MAP:
      {
        os << "{";
        const auto &a = n.as_map();
        bool separate = false;
        auto keys = a.keys();
        for (const auto &k : keys) {
          if (separate) {
            os << ", ";
          }
          else {
            os << " ";
          }
          separate = true;
          os << '"' << k << '"' << ": ";
          _stream_out(a[k], os);
        }
        if (separate) {
          os << " ";
        }
        os << "}";
      }
      return;
  }
}

json_encoder::json_encoder()
{
}

progress json_encoder::fill_buffer(
    byte *buf,
    size_t *bytecount)
{
  size_t ss_avail = m_obufend - m_obufpos;
  size_t actual_bc = std::min(*bytecount, ss_avail);
  m_obuf.read((char *)buf, actual_bc);
  m_obufpos += actual_bc;
  *bytecount = actual_bc;
  if (m_obufpos == m_obufend) {
    return progress(true);
  }
  return progress(stream_state::CONTINUE);
}

void json_encoder::reset(const node *new_node)
{
  _clear_obuf();

  _stream_out(*new_node, m_obuf);
  m_obuf.flush();
  m_obufpos = 0;
  m_obufend = m_obuf.tellp();
}


}  /* namespace ellis */
