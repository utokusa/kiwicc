#include "kiwicc.h"

/*********************************************
* ...preprocessor...
*********************************************/

// Object-like macro
typedef struct Macro Macro;
struct Macro
{
  char *name;
  Token *body;
  Macro *next;
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
  Token *tok;
};

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

static Token *push_macro(Token *tok, Macro **macros)
{
  if (tok->kind != TK_IDENT)
    error_tok(tok, "expected an identifier");
  Macro *m = calloc(1, sizeof(Macro));
  m->name = strndup(tok->loc, tok->len);
  m->body = tok->next;
  m->next = *macros;
  *macros = m;

  while (!tok->at_bol)
    tok = tok->next;

  return tok;
}

static Macro *find_macro(Token *tok, Macro *macros)
{
  char *name = strndup(tok->loc, tok->len);
  Macro *m = macros;
  while (m)
  {
    if (!strcmp(name, m->name))
      return m;
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
static void replace(Token *tok, Token *macro_body)
{
  Token *last = NULL;
  Token *next = tok->next;
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

// If tok is a macro, expand it and return true.
// If not, just return false. 
static bool expand_macro(Token **rest, Token *tok)
{
  if (tok->kind == TK_IDENT)
  {
    if (hideset_contains(tok->hideset, tok->loc, tok->len))
      return false;
    
    Macro *m = find_macro(tok, macros);
    if (m)
    {
      Hideset *hs = hideset_union(tok->hideset, new_hideset(m->name));
      Token *body = add_hideset(m->body, hs);
      replace(tok, body);
      *rest = tok;
      return true;
    }
  }

  *rest = tok;
  return false;
}

static Token *new_eof(Token *tok)
{
  Token *t = copy_token(tok);
  t->kind = TK_EOF;
  t->len = 0;
  return t;
}

// Skip until next `#endif`.
// Nested `#if` and `#endif` are skipped.
static Token *skip_cond_incl(Token *tok)
{
  while (tok->kind != TK_EOF)
  {
    if (equal(tok, "#") && equal(tok->next, "if"))
    {
      tok = skip_cond_incl(tok->next->next);
      tok = tok->next;
      continue;
    }
    if (equal(tok, "#") && equal(tok->next, "endif"))
      break;
    tok = tok->next;
  }
  return tok;
}

// Copy all tokens until the next new line.
// Copied tokens will terminated with an EOF token.
static Token *copy_line (Token **rest, Token *tok)
{
  Token head = {};
  Token *cur = &head;

  for (; !tok->at_bol; tok = tok->next)
    cur = cur->next = copy_token(tok);
  
  cur->next= new_eof(tok);
  *rest = tok;
  return head.next;
}

// Read and evaluate a constant expression
static long eval_const_expr(Token **rest, Token *tok)
{
  Token *expr = copy_line(rest, tok);
  Token *rest2;
  long val = const_expr(&rest2, expr);
  if (rest2->kind != TK_EOF)
    error_tok(rest2, "extra token");
  return val;
}

// Push `#if` to cond_incl stack
static CondIncl *push_cond_incl(Token *tok)
{
  CondIncl *ci = calloc(1, sizeof(CondIncl));
  ci->next = cond_incl;
  ci->tok = tok;
  cond_incl = ci;
  return ci;
}

Token *preprocess(Token *tok)
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

    // #include directive
    if (equal(tok, "include"))
    {
      tok = tok->next;
      if (tok->kind != TK_STR)
        error_tok(tok, "expected a string literal");
      char *file_name = tok->contents;
      char *file_path = concat(file_dir, file_name);
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
      included->next = tok = skip_line(tok->next);
      continue;
    }

    // #if directive
    if (equal(tok, "if"))
    {
      long val = eval_const_expr(&tok, tok->next);
      push_cond_incl(hash);
      if (!val)
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
  if (cond_incl)
    error_tok(cond_incl->tok, "unterminated conditional directive");
  return head.next;
}