#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Invalid number of argments.");
    return 1;
  }

  char *p = argv[1];
  int base = 10;

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %ld\n", strtol(p, &p, base));

  while (*p)
  {
    if (*p == '+')
    {
      ++p;
      printf("  add rax, %ld\n", strtol(p, &p, base));
    }
    else if (*p == '-')
    {
      ++p;
      printf("  sub rax, %ld\n", strtol(p, &p, base));
    }
    else
    {
      fprintf(stderr, "unexpected character: '%c'\n", *p);
      return 1;
    }
  }

  printf("  ret\n");
  return 0;
}
