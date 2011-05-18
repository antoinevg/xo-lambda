/* parse.c -- bootstrap lisp interpreter
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


Expression* Expression_parse(wchar_t* source, size_t source_length, Expression** symbol_list) 
{
  char buffer_utf8[source_length + 1];
  wcstombs(buffer_utf8, source, source_length);
  buffer_utf8[source_length] = 0;
  return Expression_parse_utf8(buffer_utf8, null, symbol_list);
}


char* make_substring(void* start, size_t size) 
{
  /* TODO - GC */
  char* atom = malloc(size + 1);
  atom = memset(atom, 0, size + 1);
  if (atom == null) {
    perror("Failed to allocate memory for substring.");
    exit(EXIT_FAILURE);
  }
  memcpy(atom, start, size);
  atom[size] = '\0';
  //printf("made atom: |%s|\n", atom);
  return atom;
}


typedef enum _parser_state {
  none, symbol, start_quote, start_list, end_list, start_string, delimiter
} parser_state;
char* state_names[] = {
  "none", "symbol", "start_quote", "start_list", "end_list", "start_string", "delimiter"
};
char state_table[][2] = {
  { '\'', start_quote   },
  { '(' , start_list    },
  { ')' , end_list      },
  { '\"', start_string  },
  { ' ' , delimiter     },
  { '\t', delimiter     },
  { '\r', delimiter     },
  { '\n', delimiter     },
  { 0, 0 }
}; 


parser_state token_type(char token) 
{
  for (size_t t = 0; state_table[t][0] != 0; t++) {
    if (token == state_table[t][0]) {
      return state_table[t][1];
    }
  }
  return symbol;
}


char* Expression_next_token(char** source)
{
  char* mark = *source;
  while (token_type(*mark) == delimiter) mark++;                        /* eat whitespace */
  if (token_type(*mark) != delimiter && token_type(*mark) != symbol) {  /* it's ', ( or ) */
    *source = mark + 1;
    return make_substring(mark, 1);
  }

  char* current = mark;
  while (*current != 0) {
    if (token_type(*current) != symbol) {
      *source = current;
      return make_substring(mark, current - mark);
    }
    current++;
  }

  if (current != mark) {
    return make_substring(mark, current - mark);
  }

  return null;
}


Expression* Expression_parse_string(char* source, char** last_parsed)
{
  char* peek = source;
  char* token = null;
  for (token = Expression_next_token(&peek); token != null; token = Expression_next_token(&peek)) {
    if (*token == '\"') {
      Expression* e = _new_Expression(string);
      e->car = (void*)make_substring(source, peek - source - 1);
      if (last_parsed) *last_parsed = peek;
      return e;
    }
  }
  printf("WARNING: Failed to parse string\n");

  return nil_;
}


Expression* Expression_parse_list(char* source, char** last_parsed, Expression** symbol_list)
{
  char* peek = source;
  char* token = Expression_next_token(&peek);
  if (*token == ')') {
    if (last_parsed) *last_parsed = peek;
    return nil_;
  }

  Expression* head = Expression_parse_utf8(source, &source, symbol_list);
  if (head == null) { printf("erroratom"); }

  _gc_protect(head);
  Expression* tail = Expression_parse_list(source, &source, symbol_list);
  if (tail == null) { printf("errorlist"); }

  if (last_parsed) *last_parsed = source;
  _gc_protect(tail);
  Expression* e = Expression_cons(head, tail);
  if (e == null) {  printf("errorcons");  }
  _gc_unprotect(tail); _gc_unprotect(head);

  return e;
}

// TODO - use more lisp-isms e.g. quote_ / Expression_eq etc.
Expression* Expression_parse_utf8(char* source, char** last_parsed, Expression** symbol_list)
{
  char* token = Expression_next_token(&source);
  if (last_parsed) *last_parsed = source ;
  if (token == null) { 
    return null; 
  }

  if (*token == '(') {
    Expression* e = Expression_parse_list(source, &source, symbol_list);
    if (last_parsed) *last_parsed = source;
    return e;

  } else if (*token == '\'') {
    Expression* e = Expression_parse_utf8(source, &source, symbol_list);
    if (last_parsed) *last_parsed = source;
    return Expression_cons(Expression_intern("QUOTE", symbol_list), Expression_cons(e, nil_));

  } else if (*token == ')') {
    printf("unexpected ')'\n");
    return null;

  } else if (*token == '\"') {
    Expression* e = Expression_parse_string(source, &source);
    if (last_parsed) *last_parsed = source;
    return e;
  }

  Expression* e = Expression_intern(token, symbol_list);

  return e;
}


       
