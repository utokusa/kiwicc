#include "kiwicc.h"

/*********************************************
* ...preprocessor...
*********************************************/

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
      // Null directive
      if (!equal(tok->next, "\n"))
        error_tok(tok->next, "expected a new line");
      tok = tok->next;
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
    }
  }
  return start;
}