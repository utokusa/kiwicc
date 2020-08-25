#include "kiwicc.h"

/*********************************************
* ...preprocessor...
*********************************************/

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

Token *preprocess(Token *tok)
{
  Token *start = tok;
  Token *prev = NULL;
  bool line_head = true;
  while (tok->kind != TK_EOF)
  {
    // Preprocessing directive
    if (line_head && equal(tok, "#"))
    {
      // #include directive
      if (equal(tok->next, "include"))
      {
        tok = tok->next->next;
        assert(tok->kind == TK_STR);
        char *file_name = tok->contents;
        char *file_path = concat(file_dir, file_name);
        // Tokenize
        Token *included = tokenize_file(file_path);
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
      if (!equal(tok->next, "\n"))
        error_tok(tok->next, "expected a new line");
      tok = tok->next;
      continue;
    }

    if (equal(tok, "\n"))
    {
      while (equal(tok, "\n"))
        tok = tok->next;
      if (prev)
        prev->next = tok;
      else
        start = tok;

      line_head = true;
    }
    else
    {
      prev = tok;
      tok = tok->next;

      line_head = false;
    }
  }
  return start;
}