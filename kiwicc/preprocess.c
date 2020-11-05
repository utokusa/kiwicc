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

Token *push_macro(Token *tok, Macro **macros)
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

Macro *find_macro(Token *tok, Macro *macros)
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
Token *copy_macro_body(Token *body, Token **last)
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

// Replace macro and return the last token of the macro body.
Token *replace(Token *prev, Token *tok, Macro *m)
{
  Token *last = NULL;
  prev->next = copy_macro_body(m->body, &last);
  last->next = tok->next;
  return last;
}

Token *preprocess(Token *tok)
{
  Token *start = tok;
  Token *prev = NULL;
  Macro *macros = NULL;

  while (tok->kind != TK_EOF)
  {
    // Preprocessing directive
    if (tok->at_bol && equal(tok, "#"))
    {
      // Object-like macro
      if (equal(tok->next, "define"))
      {
        tok = tok->next->next;
        if (prev)
          prev->next = push_macro(tok, &macros);
        else
          start = push_macro(tok, &macros);
        tok = prev ? prev->next : start;
        continue;
      }

      // #include directive
      if (equal(tok->next, "include"))
      {
        tok = tok->next->next;
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

        if (prev)
          prev->next = included;
        else
          start = included;

        while (included->next->kind != TK_EOF)
          included = included->next;

        prev = included;
        included->next = tok = tok->next;
        continue;
      }

      // Null directive
      if (!tok->next->at_bol)
        error_tok(tok->next, "expected a new line");
      if (prev)
        prev->next = tok->next;
      else
        start = tok->next;
      tok = tok->next;
      continue;
    }

    // Macro replacement
    if (tok->kind == TK_IDENT)
    {
      Macro *m = NULL;
      if (m = find_macro(tok, macros))
      {
        prev = replace(prev, tok, m);
        tok = prev->next;
        continue;
      }

      prev = tok;
      tok = tok->next;
      continue;
    }

    prev = tok;
    tok = tok->next;
  }
  return start;
}