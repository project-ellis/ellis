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

// TODO: think about whether this really should supplant the
// MUST_CONTINUE and MAY_CONTINUE logic of decoders.
enum class stream_status {
  MORE,
  DONE,
  ERROR,
};

class stream_progress {
  stream_status m_status;
  unique_ptr<err> m_err;

public:
  stream_progress(unique_ptr<err> e) :
    m_status(stream_status::ERROR),
    m_err(std::move(e))
  {
  }

  stream_progress(stream_status st) :
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


enum class json_tok_state {
  INIT,           /* Ready for new token. */
  STRING,         /* Inside a string. */
  ESC,            /* Quote escaping mark being applied. */
  ESC_U1,         /* Expecting 1st unicode codepoint hex digit. */
  ESC_U2,         /* Expecting 2nd unicode codepoint hex digit. */
  ESC_U3,         /* Expecting 3rd unicode codepoint hex digit. */
  ESC_U4,         /* Expecting 4th unicode codepoint hex digit. */
  NEGSIGN,        /* Just got negative sign, need digit. */
  ZERO,           /* Have initial zero (but can follow with frac or e/E). */
  INT,            /* Have non-zero int (possibly negative). */
  FRAC,           /* Have int followed by point, require frac digit. */
  FRACMORE,       /* Accepting possibly more frac digits (or e/E). */
  EXP,            /* Have e/E, require at least one digit. */
  EXPSIGN,        /* Have e/E and sign, require at least one digit. */
  EXPMORE,        /* Have e/E and digit, accepting possibly more digits. */
  COMMENTSLASH2,  /* Have first slash; waiting for second. */
  COMMENT,        /* In a // comment; waiting for newline. */
  BAREWORD,       /* Inside a bare word, might be true/false/null. */
  END,            /* Parser said completed; accept no more until reset. */
  ERROR,          /* Encountered an error; accept no more until reset. */
};

/** This emax function is a templated function meant to return the integer
 * corresponding to the maximum element in a particular simple enum class
 * (one which does not manually specify non-contiguous integral values),
 * by way of specializing the template for the particular class. */
template <typename T>
constexpr int emax();

enum class json_tok {
  LEFT_CURLY,
  RIGHT_CURLY,
  COLON,
  COMMA,
  LEFT_SQUARE,
  RIGHT_SQUARE,
  STRING,
  INTEGER,
  REAL,
  TRUE,
  FALSE,
  NIL,
  EOS,
  ERROR, /* Bad token. See also emax immediately following this enum. */
};

template <>
constexpr int emax<json_tok>() { return (int)json_tok::ERROR; }

enum class json_nts {
  VAL,
  ARR,
  ARR_CONT,
  ARR_ETC,
  MAP,
  MAP_CONT,
  MAP_PAIR,
  MAP_ETC,
  ERROR,   /* See also emax immediately following this enum. */
};

template <>
constexpr int emax<json_nts>() { return (int)json_nts::ERROR; }


template <typename TOKTYPE, typename NTSTYPE>
struct tok_nts_union {
  bool m_is_tok;
  union {
    TOKTYPE m_tok;
    NTSTYPE m_nts;
  };
  tok_nts_union(TOKTYPE x) : m_is_tok(true), m_tok(x) {}
  tok_nts_union(NTSTYPE x) : m_is_tok(false), m_nts(x) {}
  bool is_tok() const { return m_is_tok; }
  TOKTYPE tok() const { ELLIS_ASSERT(m_is_tok); return m_tok; }
  NTSTYPE nts() const { ELLIS_ASSERT(!m_is_tok); return m_nts; }
};

using json_sym = tok_nts_union<json_tok, json_nts>;

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
    m_ststat = stream_status::MORE;
  }

  unique_ptr<node> extract_node() {
    if (m_nodes.empty()) {
      return nullptr;
    }
    return unique_ptr<node>(new node(m_nodes[0]));
  }
};

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
  //{ json_nts::VAL, "VAL --> MAP",
  //  { json_sym(json_nts::MAP) },
  //  {} },
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
    { json_sym(json_tok::LEFT_SQUARE), json_sym(json_nts::ARR_CONT) },
    {} },
  { json_nts::ARR_CONT, "ARR_CONT --> ]",
    { json_sym(json_tok::RIGHT_SQUARE) },
    {} },
  { json_nts::ARR_CONT, "ARR_CONT --> VAL ARR_ETC",
    { json_sym(json_nts::VAL), json_sym(json_nts::ARR_ETC) },
    {} },
  { json_nts::ARR_ETC, "ARR_ETC --> ]",
    { json_sym(json_tok::RIGHT_SQUARE) },
    {} },
  { json_nts::ARR_ETC, "ARR_ETC --> , VAL ARR_ETC",
    { json_sym(json_tok::COMMA),
      json_sym(json_nts::VAL),
      json_sym(json_nts::ARR_ETC) },
    {} }
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
    while (1) {
      bool changes = false;
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
              changes = true;
            }
          }
        }
      }
      if (!changes) {
        break;
      }
    }
  }

public:
  explicit json_parser(const vector<json_parse_rule> &rules) :
    m_rules(rules)
  {
    _initialize_rule_matrix();
    /* Start symbol stack with basic val NTS. */
    m_state.m_syms.push_back(json_sym(json_nts::VAL));
  }

  void reset()
  {
    m_state.reset();
  }

  stream_progress progdoom(const char *msg)
  {
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
    m_state.m_ststat = stream_status::MORE;
    return stream_progress(stream_status::MORE);
  }

  stream_progress progdone()
  {
    m_state.m_ststat = stream_status::DONE;
    return stream_progress(stream_status::MORE);
  }

  /** Accept a new token, continue parsing and building the deserialized form.
   *
   * Returns true if document has been successfully built.
   */
  stream_progress accept_token(json_tok tok, const char *tokstr) {
    m_state.m_thistok = tok;
    m_state.m_thistokstr = tokstr;

    auto & stak = m_state.m_syms;
    if (stak.empty()) {
      return progdoom("parser stack underflow");
    }
    int loopMax = 2500;  // TODO: less arbitrary cycle detection
    /* Apply some rules until we consume the token. */
    for (int i = 0; i < loopMax; i++) {
      auto & top = stak.back();
      if (top.is_tok()) {
        if (top.tok() == tok) {
          /* Yay, matched the token! now move it out of the way. */
          stak.pop_back();
          /* If this was the last thing on stack, then done parsing. */
          if (stak.empty()) {
            return progdone();
          } else {
            return progmore();
          }
        } else {
          return progdoom("unexpected token");
        }
      }
      /* Top is a non-terminating symbol; lookup for this token. */
      ELLIS_ASSERT(! top.is_tok());
      auto ruleidx = m_rulematrix[(int)(top.nts())][(int)tok];
      if (ruleidx < 0) {
        return progdoom("no applicable production rule for token");
      }
      ELLIS_ASSERT(ruleidx < (int)m_rules.size());
      const auto & rule = m_rules[ruleidx];
      printf("Applying rule %d (%s)...\n", ruleidx, rule.m_desc);
      ELLIS_ASSERT(top.nts() == rule.m_lhs);
      stak.pop_back();
      /* Apply the code for this rule, if it is defined. */
      if (rule.m_fn) {
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
    m_tokstate = json_tok_state::ERROR;
    return stream_progress(unique_ptr<err>(
          new MAKE_ELLIS_ERR(err_code::TODO, msg)));
  }

  stream_progress emit_token(json_tok tok)
  {
    if (tok == json_tok::ERROR) {
      return progdoom("invalid token");
    }
    stream_progress rv = m_tokcb(tok, m_txt.str().c_str());
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
    if (str == "true") {
      return json_tok::TRUE;
    }
    else if (str == "false") {
      return json_tok::FALSE;
    }
    else if (str == "null") {
      return json_tok::NIL;
    }
    else {
      return json_tok::ERROR;
    }
    ELLIS_ASSERT_UNREACHABLE();
  }

  /** Return the overall stream progress reflecting downstream consumption
   * of tokens pursuant to node construction. */
  stream_progress accept_eos() {
    switch (m_tokstate) {
      case json_tok_state::INIT:
        return {stream_status::MORE};

      case json_tok_state::END:
        return {stream_status::DONE};

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
  }

  stream_progress start_new_token(char ch) {
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
    return {stream_status::MORE};
  }

  /** Emit the given token type, and start on a new token beginning with the
   * given character. */
  stream_progress advance_token(
      json_tok current_tok,
      char nextch)
  {
    auto rv = emit_token(current_tok);
    if (rv.stat() == stream_status::MORE) {
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
        return {stream_status::DONE};
        break;

      case json_tok_state::ERROR:
        return progdoom("prior error");
    }
    /* If we didn't return earlier, we are in the middle of a token and can
     * accept more input. */
    return {stream_status::MORE};
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
  const byte *p_end = buf + *bytecount;
  for (const byte *p = buf; p < p_end; p++) {
    auto st = m_toker->accept_char(*p);
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
  /* Send EOS to tokenizer. */
  auto st = m_toker->accept_eos();

  if (st.stat() == stream_status::ERROR) {
    if (! m_err) {
      m_err = st.extract_error();
    }
    return nullptr;
  }
  else if (st.stat() == stream_status::MORE) {
    if (! m_err) {
      m_err.reset(new MAKE_ELLIS_ERR(err_code::TODO, "no node"));
    }
    return nullptr;
  }

  return m_parser->extract_node();
}

unique_ptr<err> json_decoder::extract_error()
{
  return std::move(m_err);
}

void json_decoder::reset()
{
  m_toker.reset();
  m_parser.reset();
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
      os << (n.as_u8str());
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
          os << k << ": ";
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
