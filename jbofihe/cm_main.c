/***************************************
  $Header$

  Main routine for mini-translater
  ***************************************/

/* COPYRIGHT */

#include "cm.h"
#include "version.h"

int
yywrap(void)
{
  return 1;
}

int
main (int argc, char **argv)
{
  while (++argv,--argc) {
    if (!strcmp(*argv, "-l")) {
      do_latex = 1;
    } else if (!strcmp(*argv, "-v")) {
      fprintf(stderr, "cmafihe version %s\n", version_string);
      exit(0);
    }      
  }

  yylex();

  do_trans();

  start_output();
  do_output();
  end_output();
  output_newline();
  return 0;
}
