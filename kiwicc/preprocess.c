#include "kiwicc.h"

/*********************************************
* ...preprocessor...
*********************************************/

typedef struct MacroParam MacroParam;
struct MacroParam
{
  MacroParam *next;
  char *name;
};

typedef struct MacroArg MacroArg;
struct MacroArg
{
  MacroArg *next;
  char *name;
  Token *tok;
  bool is_last; // Used to check the number of args.
};

// Object-like macro
typedef struct Macro Macro;
struct Macro
{
  char *name;
  Token *body;
  Macro *next;
  bool is_objlike; // Object-like or function-like
  MacroParam *params;
  bool deleted;
};

typedef struct Hideset Hideset;
struct Hideset {
  Hideset *next;
  char *name;
};

// For conditional inclusion.
// `#if` can be nested, so we use a stack to manage nested `#if`s
typedef struct CondIncl CondIncl;
struct CondIncl
{
  CondIncl *next;
  enum { IN_THEN, IN_ELIF, IN_ELSE } ctx;
  Token *tok;
  bool included;
};

static Token *preprocess2(Token *tok);
static Token *copy_line(Token **rest, Token *tok);
static Token *new_eof(Token *tok);
static Macro *find_macro(Token *tok, Macro *macros);

Macro *macros = NULL;

static CondIncl *cond_incl;

// Get file directory from full file path.
// e.g. "foo/bar.txt" --> "foo/"
char *get_dir(char *path)
{
  int len = strlen(path);
  // Search for directory separator charactor '/'.
  for (int i = len - 1; i >= 0; i--)
  {
    if (i == 0)
    {
      // `path` has no directory separator charactor.
      char *dir = "./";
      return dir;
    }

    if (path[i] == '/')
    {
      char *dir = strndup(path, i + 1);
      return dir;
    }
  }
}

static char *concat(char *s1, char *s2)
{
  char *s = malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
  int i = 0;
  for (int j = 0; j < strlen(s1); j++)
    s[i++] = s1[j];
  for (int j = 0; j < strlen(s2); j++)
    s[i++] = s2[j];
  s[i] = '\0';
  return s;
}

static Token *undef_macro(Token *tok, Macro **macros)
{
  if (tok->kind != TK_IDENT)
    error_tok(tok, "expected an identifier");
  Macro *m = calloc(1, sizeof(Macro));
  m->name = strndup(tok->loc, tok->len);
  m->next = *macros;
  m->deleted = true;
  *macros = m;

  while (!tok->at_bol)
    tok = tok->next;

  return tok;
}

static MacroParam *read_macro_params(Token **rest, Token *tok)
{
  MacroParam head = {};
  MacroParam *cur = &head;

  while (!equal(tok, ")"))
  {
    if (cur != &head)
      tok = skip(tok, ",");
    
    if (tok->kind != TK_IDENT)
      error_tok(tok, "expected an identifier");
    MacroParam *m = calloc(1, sizeof(MacroParam));
    m->name = strndup(tok->loc, tok->len);
    cur = cur->next = m;
    tok = tok->next;
  }
  *rest = tok->next;
  return head.next;
}

static Token *push_macro(Token *tok, Macro **macros)
{
  if (tok->kind != TK_IDENT)
    error_tok(tok, "expected an identifier");
  Macro *m = calloc(1, sizeof(Macro));
  m->name = strndup(tok->loc, tok->len);

  tok = tok->next;

  if (!tok->has_space && equal(tok, "("))
  {
    // Function-like macro
    MacroParam *params = read_macro_params(&tok, tok->next);
    m->params = params;
    m->is_objlike = false;
  }
  else
  {
    // Object-like macro
    m->is_objlike = true;
  }

  m->body = copy_line(&tok, tok);
  m->next = *macros;
  *macros = m;

  return tok;
}

static Macro *find_macro(Token *tok, Macro *macros)
{
  char *name = strndup(tok->loc, tok->len);
  Macro *m = macros;
  while (m)
  {
    if (!strcmp(name, m->name))
      return m->deleted ? NULL : m;
    m = m->next;
  }
  return NULL;
}

// Duplicate macro body
// `last` is an output argument which represents
// the last token of the macro body
static Token *copy_macro_body(Token *body, Token **last)
{
  Token head = {};
  Token *cur = &head;
  while (body && !body->at_bol)
  {
    cur->next = copy_token(body);
    cur = cur->next;
    body = body->next;
  }
  *last = cur;
  return head.next;
}

// Replace macro
static void replace(Token *tok, Token *macro_body, Token *next)
{
  Token *last = NULL;
  Token *body_head = copy_macro_body(macro_body, &last);
  *tok = *body_head;
  if (body_head == last)
    tok->next = next;
  else
    last->next = next;
}

// Some processor directives such as #include allow extraneous
// tokens before newline. This function skips such tokens.
static Token *skip_line(Token *tok)
{
  if (tok->at_bol)
    return tok;
  warn_tok(tok, "extra token");
  while (!tok->at_bol)
    tok = tok->next;
  return tok;
}

static Hideset *new_hideset(char *name) {
  Hideset *hs = calloc(1, sizeof(Hideset));
  hs->name = name;
  return hs;
}

static Hideset *hideset_union(Hideset *hs1, Hideset *hs2) {
  Hideset head = {};
  Hideset *cur = &head;

  for (; hs1; hs1 = hs1->next)
    cur = cur->next = new_hideset(hs1->name);
  for (; hs2; hs2 = hs2->next)
    cur = cur->next = new_hideset(hs2->name);
  return head.next;
}

static bool hideset_contains(Hideset *hs, char *s, int len) {
  for (; hs; hs = hs->next)
    if (strlen(hs->name) == len && !strncmp(hs->name, s, len))
      return true;
  return false;
}

static Token *add_hideset(Token *tok, Hideset *hs) {
  Token head = {};
  Token *cur = &head;

  for (; tok ;tok = tok->next) {
    Token *t = copy_token(tok);
    t->hideset = hideset_union(t->hideset, hs);
    cur = cur->next = t;
  }
  return head.next;
}

static Hideset *hideset_intersection(Hideset *hs1, Hideset *hs2)
{
  Hideset head = {};
  Hideset *cur = &head;

  for (; hs1; hs1 = hs1->next)
  {
    if (hideset_contains(hs2, hs1->name, strlen(hs1->name)))
      cur = cur->next = new_hideset(hs1->name);
  }
  return head.next;
}

static MacroArg *read_macro_arg_one(Token **rest, Token *tok)
{
  Token head = {};
  Token *cur = &head;
  int level = 0;

  while (level > 0 || (!equal(tok, ",") && !equal(tok, ")")))
  {
    if (tok->kind == TK_EOF)
      error_tok(tok, "premature end of input");
    
    if (equal(tok, "("))
      level++;
    else if (equal(tok, ")"))
      level--;

    cur = cur->next = copy_token(tok);
    tok = tok->next;
  }

  bool is_last = equal(tok, ")");

  cur->next = new_eof(tok);

  MacroArg *arg = calloc(1, sizeof(MacroArg));
  arg->tok = head.next;
  arg->is_last = is_last;
  *rest = tok;
  return arg;
}

static MacroArg *read_macro_args(Token **rest, Token *tok, MacroParam *params)
{
  // MACRO_NAME(<args...>)
  // ^
  // start = tok
  Token *start = tok;
  // MACRO_NAME(<args...>)
  //            ^
  //            tok (points the first argument)
  tok = tok->next->next;

  MacroArg head = {};
  MacroArg *cur = &head;

  MacroParam *pp = params;
  for (; pp && !cur->is_last ; pp = pp->next)
  {
    if (cur != &head)
      tok = skip(tok, ",");
    cur = cur->next = read_macro_arg_one(&tok, tok);
    cur->name = pp->name;
  }

  if (pp)
    error_tok(start, "too many arguments");

  skip(tok, ")");
  *rest = tok;
  return head.next;
}

static Token *find_arg(MacroArg *args, Token *tok)
{
  for (MacroArg *ap = args; ap; ap = ap->next)
  {
    if (tok->len == strlen(ap->name) && !strncmp(tok->loc, ap->name, tok->len))
      return ap->tok;
  }
  return NULL;
}

static Token *stringize(Token *arg)
{
  // Count lenght of stringized string literal

  /*
  e.g.
  #define PRINT(str) printf(#str"\n")
  PRINT(   Kitty on your lap  );

  Kitty on   your lap  )            ;
  tok0  tok1 tok2 tok3 end_tok(kind == TK_EOF)
  ^                  ^
  start              end
  */

  // White space before the first preprocessing token and
  // after the last preprocessing token composing the argument is deleted.
  // In this example spaces between "(" and "Kitty" are deleted,
  // and spaces between "lap" and ")" are also deleted.

  char *start = arg->loc;
  char *end;
  Token *end_tok;
  for (Token *t = arg; t && t->kind != TK_EOF; t = t->next)
  {
    end = t->loc;
    end_tok = t;
  }
  end_tok = end_tok->next;
  // Kitty on your lap
  //               ^ end

  while (!isspace(*end) && (end < end_tok->loc))
    end++;
  // Kitty on your lap
  //                  ^ end

  // For tok->loc.
  // len is initialized with 2 for first and last `"`
  int len = 2;

  // For tok->cont_len.
  int len2 = 0;

  for (char *p = start; p != end; p++)
  {
    len++;
    len2++;
    if (*p == '\"')
      len++;
    if (*p == '\\')
      len++;
  }

  // For terminating '\0'
  len++;
  len2++;

  // For tok->loc
  char *buf = malloc(len);
  // For tok->contents
  char *buf2 = malloc(len2);

  int i = 0;
  int j = 0;
  buf[i++] = '\"';
  for (char *p = start; p != end; p++, i++, j++)
  {
    if (*p == '\"')
      buf[i++] = '\\'; 
    if (*p == '\\')
      buf[i++] = '\\';
    buf[i] = *p;
    buf2[j] = *p;
  }
  buf[len - 2] = '\"';
  buf[len - 1] = '\0';
  buf2[len2 - 1] = '\0';

  Token *tok = copy_token(arg);
  tok->loc = buf;
  tok->len = len - 1;
  tok->kind = TK_STR;
  tok->contents = buf2;
  tok->cont_len = len2 + 1; // tok->cont_len count trailing '\0'
  return tok;
  
}

static Token *paste(Token *lhs, Token *rhs)
{
  // Paste the two tokens.
  char *buf = calloc(1, lhs->len + rhs->len + 1);
  sprintf(buf, "%.*s%.*s", lhs->len, lhs->loc, rhs->len, rhs->loc);

  // Tokenize the resulting string.
  Token *tok = tokenize(lhs->filename, lhs->file_no, buf);
  if (tok->next->kind != TK_EOF)
    error_tok(lhs, "pasting forms '%s', an invalid token", buf);
  tok->at_bol = false;
  return tok;
}

// Replace func-like macro parameters with given arguments.
static Token *subst(Token *tok, MacroArg *args)
{
  Token head = {};
  Token *cur = &head;

  while (tok->kind != TK_EOF)
  {
    // # operator (stringizing operator)
    // "#" followed by a parameter is replaced with stringized actual.
    if (equal(tok, "#"))
    {
      Token *arg = find_arg(args, tok->next);
      if (!arg)
        error_tok(tok->next, "'#' is not followed by a macro parameter");

      cur = cur->next = stringize(arg);
      tok = tok->next->next;
      continue;
    }

    // ## operator (token-pasting operator)
    // x##y is replaced with xy.
    if (equal(tok->next, "##"))
    {
      Token *x = tok;
      Token *y = tok->next->next;
      Token *ax = find_arg(args, x);

      // x##y becomes y if x is the empty argument list.
      if (ax && ax->kind == TK_EOF)
      {
        tok = y;
        continue;
      }

      if (ax)
      {
        for (Token *t = ax; t->kind != TK_EOF; t = t->next)
          cur = cur->next = copy_token(t);
      }
      else
        cur = cur->next = copy_token(x);
      
      Token *ay = find_arg(args, y);

      // x##y becomes x if y is the empty argument list.
      if (ay && ay->kind == TK_EOF)
      {
        tok = y->next;
        continue;
      }

      if (ay)
      {
        *cur = *paste(cur, ay);
        for (Token *t = ay->next; t->kind != TK_EOF; t = t->next)
          cur = cur->next = copy_token(t);
      }
      else
        *cur = *paste(cur, y);
      
      tok = y->next;
      continue;
    }

    Token *arg = find_arg(args, tok);

    // Handle a macro token. Macro arguments are completely
    // macro-expanded before they are substituted into a macro body.
    if (arg)
    {
      arg = preprocess2(arg);
      for (Token *t = arg; t && t->kind != TK_EOF; t = t->next)
        cur = cur->next = copy_token(t);
      tok = tok->next;
      continue;
    }

    // Handle a non-macro token.
    cur = cur->next = copy_token(tok);
    tok = tok->next;
    continue;
  }

  cur->next = tok;
  return head.next;
}

// If tok is a macro, expand it and return true.
// In this case, assign the head of the macro body to *new_tok.
// If not, just return false and assign tok to *new_tok
static bool expand_macro(Token **new_tok, Token *tok)
{
  if (tok->kind == TK_IDENT)
  {
    if (hideset_contains(tok->hideset, tok->loc, tok->len))
      return false;
    
    Macro *m = find_macro(tok, macros);
    if (!m)
    {
      *new_tok = tok;
      return false;
    }

    // Object-like macro
    if (m->is_objlike)
    {
      Hideset *hs = hideset_union(tok->hideset, new_hideset(m->name));
      Token *body = add_hideset(m->body, hs);
      replace(tok, body, tok->next);
      *new_tok = tok;
      return true;
    }

    // Function-like macro

    if (!equal(tok->next, "("))
      return false;
    
    // If a funclike macro token is not followed by an argument list,
    // treat it as a normal identifier.

    Token *macro_name = tok;
    MacroArg *args = read_macro_args(&tok, tok, m->params);
    Token *rparen = tok;
    // e.g.
    // MACRO(<args>)<rest of the code>
    // ^ ident
    //             ^ rparen

    Hideset *hs = hideset_intersection(macro_name->hideset, rparen->hideset);
    hs = hideset_union(hs, new_hideset(m->name));

    Token *body = subst(m->body, args);
    body->at_bol = macro_name->at_bol;
    body = add_hideset(body, hs);
    replace(macro_name, body, rparen->next);
    *new_tok = macro_name;
    return true;
  }
  *new_tok = tok;
  return false;
}

static Token *new_eof(Token *tok)
{
  Token *t = copy_token(tok);
  t->kind = TK_EOF;
  t->len = 0;
  return t;
}

static Token *skip_cond_incl2(Token *tok)
{
  while (tok->kind != TK_EOF)
  {
    if (equal(tok, "#") && (equal(tok->next, "if") ||
        equal(tok->next, "ifdef") || equal(tok->next, "ifndef")))
    {
      tok = skip_cond_incl2(tok->next->next);
      continue;
    }

    if (equal(tok, "#") && equal(tok->next, "endif"))
      return tok->next->next;
    
    tok = tok->next;
  }
  return tok;
}

// Skip until next `#elif`, `#else` or `#endif`.
// Nested `#if` and `#endif` are skipped.
static Token *skip_cond_incl(Token *tok)
{
  while (tok->kind != TK_EOF)
  {
    if (equal(tok, "#") && (equal(tok->next, "if") ||
        equal(tok->next, "ifdef") || equal(tok->next, "ifndef")))
    {
      tok = skip_cond_incl2(tok->next->next);
      continue;
    }
    if (equal(tok, "#") && ( equal(tok->next, "elif")
        || equal(tok->next, "else") || equal(tok->next, "endif")))
      break;
    tok = tok->next;
  }
  return tok;
}

// Copy all tokens until the next new line.
// Copied tokens will terminated with an EOF token.
static Token *copy_line(Token **rest, Token *tok)
{
  Token head = {};
  Token *cur = &head;

  for (; !tok->at_bol; tok = tok->next)
    cur = cur->next = copy_token(tok);
  
  cur->next= new_eof(tok);
  *rest = tok;
  return head.next;
}

static Token *new_num_token(int val, Token *tmpl)
{
  char *buf = calloc(1, 30);
  sprintf(buf, "%d\n", val);
  return tokenize(tmpl->filename, tmpl->file_no, buf);
}

static Token *read_const_expr(Token **rest, Token *tok)
{
  tok = copy_line(rest, tok);

  Token head = {};
  Token *cur = &head;

  while (tok->kind != TK_EOF)
  {
    // defined() macro operator
    // "defined foo" and "defined (foo)" are both expressions whose value is 1 
    // if name is defined as a macro at the current point in the program, and 0 otherwise. 

    if (equal(tok, "defined"))
    {
      Token *start = tok;
      bool has_paren = equal(tok->next, "(");
      if (has_paren)
        tok = tok->next->next;
      else
        tok = tok->next;
      
      if (tok->kind != TK_IDENT)
        error_tok(start, "macro name must be an identifier");
      Macro *m = find_macro(tok, macros);
      tok = tok->next;

      if (has_paren)
        tok = skip(tok, ")");
      
      cur = cur->next  = new_num_token(m ? 1 : 0, start);
      continue;
    }

    cur = cur->next = tok;
    tok = tok->next;
  }

  cur->next = tok;
  return head.next;
}

// Read and evaluate a constant expression
static long eval_const_expr(Token **rest, Token *tok)
{
  Token *expr = read_const_expr(rest, tok);
  expr = preprocess2(expr);

  // Replace remaining non-macro identifieres with 0.
  for (Token *t = expr; t->kind != TK_EOF; t= t->next)
  {
    if (t->kind == TK_IDENT)
    {
      Token *next = t->next;
      *t = *new_num_token(0, t);
      t->next = next;
    }
  }
  Token *rest2;
  long val = const_expr(&rest2, expr);
  if (rest2->kind != TK_EOF)
    error_tok(rest2, "extra token");
  return val;
}

// Push `#if` to cond_incl stack
static CondIncl *push_cond_incl(Token *tok, bool included)
{
  CondIncl *ci = calloc(1, sizeof(CondIncl));
  ci->next = cond_incl;
  ci->ctx = IN_THEN;
  ci->tok = tok;
  ci->included = included;
  cond_incl = ci;
  return ci;
}

// Concatenates all tokens in `tok` and returns a new string.
static char *join_tokens(Token *tok, Token *end)
{
  // Compute the length of the resulting token.
  int len = 1;
  for (Token *t = tok; t != end && t->kind != TK_EOF; t = t->next)
  {
    if (t != tok && t->has_space)
      len++;
    len += t->len;
  }

  char *buf = calloc(1, len);

  // Copy token texts.
  int pos = 0;
  for (Token *t = tok; t != end && t->kind != TK_EOF; t = t->next)
  {
    if (t != tok && t->has_space)
      buf[pos++] = ' ';
    strncpy(buf + pos, t->loc, t->len);
    pos += t->len;
  }
  buf[pos] = '\0';
  return buf;
}

// Returns true if a given file exists.
static bool file_exists(char *path)
{
  struct stat st;
  return !stat(path, &st);
}

// Read an #include argument.
static char *read_include_path(Token **rest, Token *tok)
{
  // Pattern 1: #include "foo.h"
  if (tok->kind == TK_STR)
  {
    // A double-quoted filename for #include is a special kind of token,
    // and we don't want to interpret any escape sequances in it.
    // So we don't use token->contents.

    char *filename = strndup(tok->loc + 1, tok->len - 2);
    *rest = skip_line(tok->next);
    // *rest = tok->next;
    char *path = concat(file_dir, filename);
    return path;
  }

  // Pattern 2: #include <foo.h>
  if (equal(tok, "<"))
  {
    // Reconstruct a filename from a sequence of tokens between "<" and ">".
    Token *start = tok;

    // Find closing ">".
    for (; !equal(tok, ">"); tok = tok->next)
    {
      if (tok->kind == TK_EOF)
        error_tok(tok, "expected '>'");
    }

    char *filename = join_tokens(start->next, tok);
    *rest = skip_line(tok->next);

    // Search a file from the include path.
    // TODO: implementthe actual include paths.
    char *path = concat("./tests/", filename);
    if (!file_exists(path))
      error_tok(start, "'%s': file not found", filename);
    return path;
  }

  // Pattern 3 #include FOO
  // In this case FOO must be macro-expanded to either
  // a single string token or a sequence of "<" ... ">".
  if (tok->kind == TK_IDENT)
  {
    Token *tok2 = preprocess(copy_line(rest, tok));
    return read_include_path(&tok2, tok2);
  }

  error_tok(tok, "expected a filename");
}

static Token *preprocess2(Token *tok)
{
  Token head = {};
  Token *cur = &head;

  while (tok && tok->kind != TK_EOF)
  {
    // Macro replacement
    if (expand_macro(&tok, tok))
    {
      cur->next = tok;
      continue;
    }
    // Preprocessing directive
    if (!tok->at_bol || !equal(tok, "#"))
    {
      cur = cur->next = tok;
      tok = tok->next;
      continue;
    }

    // hash->loc: "#{some string ...}"
    Token *hash = tok;
    tok = tok->next;

    // Object-like macro
    if (equal(tok, "define"))
    {
      tok = tok->next;
      tok = cur->next = push_macro(tok, &macros);
      continue;
    }

    // #undef directive
    if (equal(tok, "undef"))
    {
      tok = tok->next;
      tok = cur->next = undef_macro(tok, &macros);
      continue;
    }

    // #include directive
    if (equal(tok, "include"))
    {
      char *file_path = read_include_path(&tok, tok->next);
      // Tokenize
      Token *included = tokenize_file(file_path);
      if (!included)
        error_tok(tok, "%s", strerror(errno));
      // Preprocess
      included = preprocess(included);

      if (included->kind == TK_EOF)
        continue;

      cur->next = included;

      while (included->next->kind != TK_EOF)
        included = included->next;

      cur = included;
      included->next = tok;
      continue;
    }

    // #if directive
    if (equal(tok, "if"))
    {
      long val = eval_const_expr(&tok, tok->next);
      push_cond_incl(hash, val);
      if (!val)
        tok = skip_cond_incl(tok);
      continue;
    }

    // #ifdef directive
    if (equal(tok, "ifdef"))
    {
      bool defined = find_macro(tok->next, macros);
      push_cond_incl(tok, defined);
      tok = skip_line(tok->next->next);
      if (!defined)
        tok = skip_cond_incl(tok);
      continue;
    }

    // #ifndef directive
    if (equal(tok, "ifndef"))
    {
      bool defined = find_macro(tok->next, macros);
      push_cond_incl(tok, !defined);
      tok = skip_line(tok->next->next);
      if (defined)
        tok = skip_cond_incl(tok);
      continue;
    }

    // #elif directive
    if (equal(tok, "elif"))
    {
      if (!cond_incl || cond_incl->ctx == IN_ELSE)
        error_tok(hash, "stray #elif");
      cond_incl->ctx = IN_ELIF;

      if (!cond_incl->included && eval_const_expr(&tok, tok->next))
        cond_incl->included = true;
      else
        tok = skip_cond_incl(tok);
      continue;
    }

    // #else directive
    if (equal(tok, "else"))
    {
      if (!cond_incl || cond_incl->ctx == IN_ELSE)
        error_tok(hash, "stray #else");
      cond_incl->ctx = IN_ELSE;
      tok = skip_line(tok->next);

      if (cond_incl->included)
        tok = skip_cond_incl(tok);
      continue;
    }

    // #endif directive
    if (equal(tok, "endif"))
    {
      if (!cond_incl)
        error_tok(hash, "stray #endif");
      cond_incl = cond_incl->next;
      tok = skip_line(tok->next);
      continue;
    }

    // Null directive
    if (!tok->at_bol)
      error_tok(tok->next, "expected a new line");
    continue;
  }
  return head.next;
}

Token *preprocess(Token *tok)
{
  tok = preprocess2(tok);
  if (cond_incl)
    error_tok(cond_incl->tok, "unterminated conditional directive");
  return tok;
}