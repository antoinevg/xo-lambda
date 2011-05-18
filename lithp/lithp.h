/* lithp.h -- bootstrap lisp interpreter
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

#ifndef LITHP_H
#define LITHP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <phlo.h>

typedef enum _type {
  list, atom, string, pointer, foreign, number, integer, real // TODO - differentiate between naturals and integers
} ExpressionType;


typedef struct _Expression {
  struct _Expression* car; /* expression := atom | expression (first) */
  struct _Expression* cdr; /* expression                      (rest)  */
  unsigned type  : 4;      /* list | atom | string | pointer | foreign | number | integer | real */
  unsigned mark  : 1;      /* used by garbage collector               */
  unsigned flags : 3;      
  char pad_to_word_boundary[3];
} Expression; 


/* utilities */
bool is_a_number(const char* s);
double to_double(Expression* n);
ExpressionType guess_numeric_type(const char* s);



/* TODO - we have some static data, this is it */
/* static */ Expression* t_;
/* static */ Expression* nil_;
/* static */ Expression* quote_;
/* static */ Expression* atom_;
/* static */ Expression* eq_;
/* static */ Expression* car_;
/* static */ Expression* cdr_;
/* static */ Expression* cons_;
/* static */ Expression* cond_;
/* static */ Expression* label_;
/* static */ Expression* lambda_;
/* static */ Expression* progn_;

/* static */ Expression* set_;
/* static */ Expression* assoc_;
/* static */ Expression* print_;
/* static */ Expression* quit_;
/* static */ Expression* reverse_;


/* static */ Expression* gt_;
/* static */ Expression* lt_;
/* static */ Expression* gte_;
/* static */ Expression* lte_;
/* static */ Expression* add_;
/* static */ Expression* sub_;
/* static */ Expression* mul_;
/* static */ Expression* div_;

/* static */ Expression* xl_gc_;
/* static */ Expression* xl_environment_;
/* static */ Expression* xl_symbol_list_;
/* static */ Expression* xl_dlopen_;
/* static */ Expression* xl_dlclose_;
/* static */ Expression* xl_dlsym_;



typedef struct {
  Expression*  heap;
  unsigned int heap_size;
  Expression*  free_list;
  unsigned int free_count;
  Expression** protect_stack;
  unsigned int protect_size;
  unsigned int protect_top;
} GC;
GC* new_GC(void);
void delete_GC(GC* self);
void _gc_protect(Expression* e);
void _gc_unprotect(Expression* e);
void _gc_mark(Expression* e, char symbol);
Expression* _gc_collect(void);
Expression* _new_Expression(ExpressionType t);  // functions with _ in front mess with static data
/* static */ //Expression* _symbol_list;
/* static */ //Expression* _environment;

typedef struct {
  GC* gc;
  Expression* symbol_list;
  Expression* environment;
} LithpInterpreter;
/* TODO - rewrite to allow multiple lithp interpreter instances */
/* static */ GC* _global_gc;
/* static */ LithpInterpreter* _global_interpreter;
LithpInterpreter* new_LithpInterpreter(void);
void delete_LithpInterpreter(LithpInterpreter* self);


Expression* Expression_parse(wchar_t* source, size_t source_length, Expression** symbol_list);
Expression* Expression_parse_utf8(char* source, char** last_parsed, Expression** symbol_list);
Expression* Expression_intern(char* symbol, Expression** symbol_list);


/* forward declarations  -  just this, along with the linked list data structure is lisp. */
Expression* Expression_quote (Expression* expression);
Expression* Expression_atom  (Expression* expression);
Expression* Expression_eq    (Expression* x, Expression* y);
Expression* Expression_car   (Expression* expression);
Expression* Expression_cdr   (Expression* expression);
Expression* Expression_cons  (Expression* x, Expression* y);
Expression* Expression_cond  (Expression* c, Expression* a); 
Expression* Expression_eval  (Expression* e, Expression* a);
Expression* Expression_evlis (Expression* m, Expression* a);
Expression* Expression_lambda(Expression* l, Expression* a);
Expression* Expression_progn (Expression* x, Expression* y);
Expression* Expression_null  (Expression* x);
Expression* Expression_list  (Expression* x, Expression* y);
Expression* Expression_append(Expression* x, Expression* y);
Expression* Expression_pair  (Expression* x, Expression* y);
Expression* Expression_assoc (Expression* x, Expression* y);


Expression* Expression_print(Expression* expression);
Expression* Expression_reverse(Expression* list);

void Expression_dump(Expression* expression, size_t indent);

#include <wchar.h>
void Expression_to_string(Expression* expression, wchar_t* buffer, size_t buffer_length);

/* math */
Expression* Expression_gt(Expression* x, Expression* y);
Expression* Expression_lt(Expression* x, Expression* y);
Expression* Expression_gte(Expression* x, Expression* y);
Expression* Expression_lte(Expression* x, Expression* y);
Expression* Expression_add(Expression* x, Expression* y);
Expression* Expression_sub(Expression* x, Expression* y);
Expression* Expression_mul(Expression* x, Expression* y);
Expression* Expression_div(Expression* x, Expression* y);


/* FFI */
Expression* Expression_dlopen(Expression* path);
Expression* Expression_dlclose(Expression* handle);
Expression* Expression_dlsym(Expression* handle, Expression* symbol);
Expression* Expression_call(Expression* e);

#endif // LITHP_H
