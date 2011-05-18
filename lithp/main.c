/* main.c -- bootstrap lisp interpreter
 *
 * Copyright (c) 2008 Antoine van Gelder
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, provided that the above copyright notice(s) and this
 * permission notice appear in all copies of the Software and that both the
 * above copyright notice(s) and this permission notice appear in supporting
 * documentation.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 */

#include "lithp.h"


/**
 * read
 *
 * (read stream buffer length) reads a sexp from stdin 
 *
 * TODO - add some state awareness so we return sexps and not just lines 
 */
wchar_t* Read(FILE* stream, wchar_t* buffer, size_t buffer_length) 
{
  size_t bracket_count = 0;
  size_t character_count = 0;
  bool in_comment = false;
  for (size_t t = 0; t < buffer_length - 1; t++) {
    wint_t c = fgetwc(stream);
    buffer[t] = c;
    buffer[t + 1] = 0;
    switch (c) {
      case EOF :         return null;
      case ' ':          if (character_count == 0) t--; continue;
      case '(' :         bracket_count++; continue;
      case ')' :         bracket_count--; continue;
      case ';' :         in_comment = true; continue;
      case '\n':
        if (in_comment) {
          in_comment = false;
          t = -1;
          continue;
        } else if (bracket_count != 0) {
          //printf("Brackets: %d\n", bracket_count);
          continue;
        } else if (t <= 0) {
          t--;
          continue;
        } else if (character_count == 0) {
          t--;
          continue;
        }
        buffer[t] = 0;
        goto done;
      default:  
        character_count++;
    }
  }
 done:
  //printf("GOT: |%S|\n", buffer); fflush(stdout);
  return buffer;
}



/**
 * main
 */
int main(int argc, char** argv)
{
  /* Initialization */
  LithpInterpreter* lithp = new_LithpInterpreter();
  FILE* stream = stdin;
  if (argc == 2) {
    printf("Opening %s\n", argv[1]);
    stream = fopen(argv[1], "r");
    if (stream == null) {
      perror("Could not open file for reading");
      return EXIT_FAILURE;
    }
  }

  size_t buffer_length = 1024;
  wchar_t buffer[buffer_length];  

  while (true) {
    //printf("h.[%d/%d] p.[%d/%d] . ", lithp->gc->free_count, lithp->gc->heap_size, lithp->gc->protect_top, lithp->gc->protect_size);             
    printf(". ");
    if (Read(stream, buffer, buffer_length) == null) {            /* -> Read  */                                   
      break;
    }
    if (wcslen(buffer) <= 1) { continue; } /* TODO - vulnerability, modify Read to return number of chars read */
    Expression* expression = Expression_parse(buffer, buffer_length, &lithp->symbol_list);
    if (expression == null) {
      printf("Error - Invalid expression: ");
      wprintf(L"%ls\n", buffer);
      continue;
    }

    _gc_protect(expression);
    Expression* ret = Expression_eval(expression, lithp->environment); /* -> Eval  */
    _gc_unprotect(expression);
    if (ret == null) {
      printf("Quitting...");
      break;
    }
    
    //Expression_dump(ret, -1);                                 /* -> Print */                    
    memset(buffer, 0, sizeof(buffer));
    Expression_to_string(ret, buffer, buffer_length);
    //wprintf(L"%ls\n", buffer);               /* TODO */     
    printf("%S\n", buffer);                    

    //_free_list = _gc_collect();
  }                                                              /* -> Loop  */

  if (stream != stdin) {
    if (fclose(stream) != 0) {
      perror("Could not close stream");
    }
  }

  printf("Goodbye!\n");

  return EXIT_SUCCESS;
}


