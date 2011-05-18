/* lithp.c -- bootstrap lisp interpreter
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


/* Utilities */
bool is_a_number(const char* s) {
  for (size_t t = 0; t < strlen(s); t++) {
    if (s[t] == '.')         { continue;     }
    if (isdigit(s[t]) == 0) { return false; }
  }
  return true;
}

// TODO - add support for natural numbers vs integers
ExpressionType guess_numeric_type(const char* s)
{
  for (size_t t = 0; t < strlen(s); t++) {
    if (s[t] == '.')         { return real;     }
  }
  return integer;
}



/**
 * Bootstrap
 */
LithpInterpreter* new_LithpInterpreter()
{
  LithpInterpreter* self = _global_interpreter = malloc(sizeof(LithpInterpreter));  
  bzero(_global_interpreter, sizeof(LithpInterpreter));
  //self->symbol_list = null;
  //self->environment = null;
  self->gc = _global_gc = new_GC(); 
  /* Bootstrap */
  /* TODO - do we want to share environments and symbol lists over
            multiple interpreter instances ? */            
  nil_ = _new_Expression(list);
  nil_->car = (void*)"NIL";
  nil_->cdr = nil_;
  self->symbol_list = Expression_cons(nil_, nil_);
  t_       = Expression_intern("T",      &self->symbol_list); 
  quote_   = Expression_intern("QUOTE",  &self->symbol_list); 
  atom_    = Expression_intern("ATOM",   &self->symbol_list); 
  eq_      = Expression_intern("EQ",     &self->symbol_list); 
  car_     = Expression_intern("CAR",    &self->symbol_list);
  cdr_     = Expression_intern("CDR",    &self->symbol_list); 
  cons_    = Expression_intern("CONS",   &self->symbol_list);
  cond_    = Expression_intern("COND",   &self->symbol_list); 
  label_   = Expression_intern("LABEL",  &self->symbol_list); 
  lambda_  = Expression_intern("LAMBDA", &self->symbol_list); 
  progn_   = Expression_intern("PROGN",  &self->symbol_list); 
  set_     = Expression_intern("SET",    &self->symbol_list); 
  assoc_   = Expression_intern("ASSOC",  &self->symbol_list); 
  print_   = Expression_intern("PRINT",  &self->symbol_list);
  reverse_ = Expression_intern("REVERSE",&self->symbol_list);
  quit_    = Expression_intern("QUIT",   &self->symbol_list);
  gt_      = Expression_intern("GT",     &self->symbol_list);
  lt_      = Expression_intern("LT",     &self->symbol_list);
  gte_     = Expression_intern("GTE",    &self->symbol_list);
  lte_     = Expression_intern("LTE",    &self->symbol_list);
  add_     = Expression_intern("ADD",    &self->symbol_list); 
  sub_     = Expression_intern("SUB",    &self->symbol_list); 
  mul_     = Expression_intern("MUL",    &self->symbol_list); 
  div_     = Expression_intern("DIV",    &self->symbol_list); 
  xl_gc_          = Expression_intern("xl:gc",          &self->symbol_list);
  xl_environment_ = Expression_intern("xl:environment", &self->symbol_list);
  xl_symbol_list_ = Expression_intern("xl:symbol-list", &self->symbol_list);
  xl_dlopen_      = Expression_intern("xl:dlopen",      &self->symbol_list);
  xl_dlclose_     = Expression_intern("xl:dlclose",     &self->symbol_list);
  xl_dlsym_       = Expression_intern("xl:dlsym",       &self->symbol_list);
  Expression* l = Expression_list(t_, nil_);
  self->environment = Expression_pair(l, l);
  return self;
}

void delete_LithpInterpreter(LithpInterpreter* self) 
{
  /* TODO */
}


GC* new_GC() 
{
  GC* self = malloc(sizeof(GC));
  bzero(self, sizeof(GC));
  self->heap = null;
  self->heap_size = 1024;
  self->free_list = null;
  self->protect_stack = null;
  self->protect_size = 512;
  self->protect_top = 0;
  return self;
}
void delete_GC(GC* self) 
{
}



/**
 *  From Flake:
 *   1. mark the contents of the environment
 *   2. mark the contents of the symbol table ?
 *   3. mark the contents of the protect table (like stutter ?)
 *   4. add every unmarked cell left on the heap to the free list
 *
 * . Allocation happens from the free list.
 * . Heap is expanded when the free list is empty after a collection
 */
//#define DEBUG_GC true 
void _gc_unprotect(Expression* e) { 
#ifdef DEBUG_GC
  Expression* release = _global_gc->protect_stack[_global_gc->protect_top - 1];
  if (release != e) {
    printf("Protection mismatch: Releasing |"); Expression_dump(e, -1); printf("|");
    printf(" Top |"); Expression_dump(release, -1); printf("|\n");
  }
#endif /* DEBUG_GC */
    _global_gc->protect_top--; 
}
void _gc_protect(Expression* e) 
{ 
  //printf("[%d/%d]",  _global_gc->protect_top, _global_gc->protect_size);             
  if (_global_gc->protect_top >= _global_gc->protect_size) {
#ifdef DEBUG_GC
    fprintf(stderr, "Expanding protect-stack\n");
#endif /* DEBUG_GC */
    _global_gc->protect_stack = realloc(_global_gc->protect_stack, 
                                        sizeof(Expression*) * _global_gc->protect_size * 2);
    _global_gc->protect_size *= 2;
  }
  _global_gc->protect_stack[_global_gc->protect_top++] = e;
}
void _gc_mark(Expression* e, char symbol) 
{
  if (e == null || e->mark) {
    return;
  }
#ifdef DEBUG_GC
  //fprintf(stderr, "%c@(%p) ", symbol, e); 
  fprintf(stderr, "%c", symbol); 
#endif /* DEBUG_GC */
  e->mark = 1;
  if (e->type == list && e != null && e != nil_) {
    _gc_mark(e->car, toupper(symbol));
    _gc_mark(e->cdr, toupper(symbol));
  }
}
Expression* _gc_collect()
{
#ifdef DEBUG_GC 
  fprintf(stderr, "\n\nCollection");
#endif /* DEBUG_GC */

  // mark the roots
#ifdef DEBUG_GC 
  fprintf(stderr, "->roots");
#endif /* DEBUG_GC */
  _gc_mark(_global_interpreter->symbol_list, 's');
  _gc_mark(_global_interpreter->environment, 'e');
  for (size_t t = 0; t < _global_gc->protect_top; t++) {
    _gc_mark(_global_gc->protect_stack[t], 'p');
  }
#ifdef DEBUG_GC 
  fprintf(stderr, "->releasing->");
#endif /* DEBUG_GC */

  Expression* freed = null;
  _global_gc->free_count = 0;
  Expression* tail = _global_gc->heap;
  for (size_t t = 0; t < _global_gc->heap_size - 1; t++) {
    if (tail->mark == 0) {
      char c = '.';
      if (tail->type == integer) {
        c = 'i';
#ifdef DEBUG_GC
        //        fprintf(stderr, "[%d@(%p)]", *((int*)tail->car), tail->car);
#endif /* DEBUG_GC */
        free((int*)tail->car); 
      } else if (tail->type == real) {
        c = 'r';
#ifdef DEBUG_GC
        //        fprintf(stderr, "[%f@(%p)]", *((double*)tail->car), tail->car);
#endif /* DEBUG_GC */
        free((double*)tail->car); 

      } else if (tail->type == string) {
        c = 's';
#ifdef DEBUG_GC
        //        fprintf(stderr, "[\"%s\"@(%p)]", tail->car, tail->car);
#endif /* DEBUG_GC */
        free(tail->car);  
      } 
#ifdef DEBUG_GC
      fprintf(stderr, "%c", c);
#endif /* DEBUG_GC */
      tail->type = atom;
      tail->car = freed;
      //tail->cdr = null;
      freed = tail;
      _global_gc->free_count++;
    } else {
#ifdef DEBUG_GC
      fprintf(stderr, "#");
#endif /* DEBUG_GC */
    }
    tail->mark = 0;
    tail++;
  }
#ifdef DEBUG_GC
  fprintf(stderr, ".collected %d nodes\n\n", _global_gc->free_count);
#endif /* DEBUG_GC */
  //fprintf(stderr, "[collected.%d]", _global_gc->free_count);
  return freed;
}


Expression* _new_Expression(ExpressionType t)
{
  if (_global_gc->heap == null) {
    _global_gc->heap = malloc(sizeof(Expression) * _global_gc->heap_size);
    if (_global_gc->heap == null) {
      perror("Failed to allocate memory for gc heap");
      exit(EXIT_FAILURE);
    } 
    bzero(_global_gc->heap, sizeof(Expression) * _global_gc->heap_size); /* TODO - proper construction of individual Expressions ? */
    Expression* tail = _global_gc->heap;
    for (size_t t = 0; t < _global_gc->heap_size - 1; t++) {
      tail->mark = 0;
      tail->car = tail + 1;
      tail = tail->car;
    }
    tail->car = null;
    _global_gc->protect_stack = malloc(sizeof(Expression*) * _global_gc->protect_size);
    if (_global_gc->protect_stack == null) {
      perror("Failed to allocate memory for garbage collector.");
      exit(EXIT_FAILURE);
    }
    bzero(_global_gc->protect_stack, sizeof(Expression*) * _global_gc->protect_size);
  }
  if (_global_gc->free_list == null || _global_gc->free_list == nil_) {
    _global_gc->free_list = _gc_collect();
  }
  Expression* self = _global_gc->free_list;
  if (self == null || self == nil_) { printf("The very very bad thing happened: %p\n", self); }
  self->mark = 0;
  _global_gc->free_list = _global_gc->free_list->car;

  //Expression* self = malloc(sizeof(Expression));
  self->car = nil_;
  self->cdr = nil_;
  self->type = t;

  return self;
}



void delete_Expression(Expression* self)
{
  if (self == null)        { return;           }

  // we don't want to nuke interned expressions
  if ((self->type == string || self->type >= number) && self->car != null) {
    free(self->car);
  }

  free(self);
}





/**
 * The rules for printing lisp expressions are really simple:
 *
 *   . If the expression is a string, print " then the characters in the string, then another ".
 *   . If the expression is a symbol, print the characters in the symbol's name.
 *   . If the expression is a number, print the digits representing that numbers (base 10).
 *   . If the expression is a list, print a left parenthesis, then print each element in the list, 
 *     with a space after each element but the last, then print a right parenthesis.
 *
 * (From: http://www.cs.northwestern.edu/academics/courses/325/readings/)
 */
void Expression_dump(Expression* expression, size_t indent)
{
  if (Expression_null(expression) == t_) {
    printf("NIL");
  } else if (Expression_atom(expression) == t_) {
    switch (expression->type) {
    case(atom):    printf("%s", expression->car ? (char*)expression->car : "null.atom"); break;
    case(number):  printf("WTF-%d", *expression->car); break;
    case(integer): printf("%d(d)", *expression->car); break;
    case(real):    printf("%f(f)", *expression->car); break;
    case(string):  printf("\"%s\"", expression->car ? (char*)expression->car : "null.string"); break;
    case(pointer): printf("%p", expression->car ? expression->car : null); break;
    case(foreign): printf("%p()", expression->car ? expression->car : null); break;
    default: printf("unknown");
    }
  } else {

    if (indent != -1) {
      if (indent != 0) { printf("\n"); }
      for (size_t t = 0; t < indent; t++) { printf(" "); }
      indent += 2;
    }

    printf("(");
    while (expression->type == list && expression != nil_) {  // TODO - we're getting expressions that are != lists here when we do the label
      Expression_dump(expression->car, indent);
      expression = expression->cdr;
      printf((expression != nil_ && expression->type == list) ? " " : "");
    }
    printf(")");
  }
}


/**
 * Convert expression to a string
 */
void Expression_to_string(Expression* expression, wchar_t* buffer, size_t buffer_length)
{
  if (Expression_null(expression) == t_) {
    wcsncat(buffer, L"NIL", buffer_length);
  } else if (Expression_atom(expression) == t_) { /* TODO - handle n > print_buffer_length in swprintf */
    size_t print_buffer_length = 256;
    wchar_t print_buffer[print_buffer_length];
    switch (expression->type) {
    case(atom): 
      swprintf(print_buffer, print_buffer_length, 
               L"%s", expression->car ? (char*)expression->car : "null.atom"); 
      break;
    case(number): 
      swprintf(print_buffer, print_buffer_length,
               L"WTF-%f", *expression->car); 
      break;
    case(integer): 
      swprintf(print_buffer, print_buffer_length,
               L"%d", *expression->car); 
      break;
    case(real): 
      swprintf(print_buffer, print_buffer_length,
               L"%f", *expression->car); 
      break;
    case(string): 
      swprintf(print_buffer, print_buffer_length,
               L"\"%s\"", expression->car ? (char*)expression->car : "null.string"); 
      break;
    case(pointer):
      swprintf(print_buffer, print_buffer_length,
               L"POINTER %p", expression->car);
      break;
    case(foreign):
      swprintf(print_buffer, print_buffer_length,
               L"FOREIGN-FUNCTION %p", expression->car);
      break;
    default: 
      swprintf(print_buffer, print_buffer_length, 
               L"unknown-expression-type", buffer_length); 
    }
    wcsncat(buffer, print_buffer, buffer_length);
  } else {
    wcsncat(buffer, L"(", buffer_length);
    while (expression->type == list && expression != nil_) {  // TODO - we're getting expressions that are != lists here when we do the label
      Expression_to_string(expression->car, buffer, buffer_length);
      expression = expression->cdr;
      wcsncat(buffer, (expression != nil_ && expression->type == list) ? L" " : L"", buffer_length);
    }
    wcsncat(buffer, L")", buffer_length);
  }
}




/**
 * intern
 *
 * (intern x y) returns the symbol with the name x in environment y, creating it if
 * necessary.
 * 
 * This version also detects and makes numbers and strings.
 *
 * is the environment and the symbol table the same thing ?
 * shouldn't I be appending this to... ?
 * 
 *
 * Destructive. It appends things to the environment
 */
Expression* Expression_intern(char* symbol, Expression** symbol_list)
{
  // TODO - parse strings properly, also fix parser to handle strings properly 
  if (symbol[0] == '\"' && symbol[strlen(symbol) - 1] == '\"') { 
    printf("interning string: |%s|\n", symbol);  // TODO - can we lose this ?
    Expression* s = _new_Expression(string);
    s->car = (void*)symbol;
    return s;

  } else if (is_a_number(symbol)) {
    ExpressionType type = guess_numeric_type(symbol);
    Expression* n = _new_Expression(type);
    if (type == real) {
      double* i = malloc(sizeof(double));
      if (i == null) {
        perror("Failed to allocate memory for numeric type");
        exit(EXIT_FAILURE);
      }
      *i = atof(symbol);
      n->car = (void*)i;
    } else { /* integer */
      int* i = malloc(sizeof(int));
      if (i == null) {
        perror("Failed to allocate memory for numeric type");
        exit(EXIT_FAILURE);
      }
      *i = atoi(symbol);
      n->car = (void*)i;
    } 
    return n;
  } 

  // look in symbol table
  Expression* tail = *symbol_list;
  while (tail != nil_) { 
    if (strcasecmp(symbol, (char*)tail->car->car) == 0) {
      return tail->car;
    }
    tail = tail->cdr;
  }

  // make a new one and append it to the symbol list
  Expression* e = _new_Expression(atom);
  e->car = (void*)symbol;
  *symbol_list = Expression_cons(e, *symbol_list);

  return e;
}



/**
 * null
 *
 * (null x) tests whether the argument is the empty list.
 *
 * (defun null (x)
 *   (eq x '()))
 */
Expression* Expression_null(Expression* x)
{
  return (Expression_eq(x, nil_));
}



/**
 * 1. quote
 *
 * (quote x) returns x. For readability we may abbreviate (quote x) as 'x
 *
 *   > (quote a) 
 *   a 
 *   > 'a 
 *   a 
 *   > (quote (a b c)) 
 *   (a b c) 
 */
Expression* Expression_quote(Expression* x)
{
  return x;
}


/**
 * 2. atom
 *
 * (atom x) returns the atom t if the value of x is an atom or the empty 
 * list. Otherwise it returns (). In Lisp we conventionally use the atom t to 
 * represent truth, and the empty list to represent falsity.
 *
 *   > (atom 'a) 
 *   t 
 *   > (atom '(a b c)) 
 *   () 
 *   > (atom '()) 
 *   t 
 */
Expression* Expression_atom(Expression* x)
{
  return ((Expression_eq(x, nil_) == t_) || (x->type != list)) ? t_ : nil_;
}


/** 
 * 3. eq
 *
 * (eq x y) returns t if the values of x and y are the same atom or both the 
 * empty list, and () otherwise.
 *
 *   > (eq 'a 'a)
 *   t 
 *   > (eq 'a 'b) 
 *   ()
 *   > (eq '() '()) 
 *   t 
 */
Expression* Expression_eq(Expression* x, Expression* y)
{
  if (x == null || y == null) { printf("EQNULL\n"); x = nil_; } // TODO - get rid of null
  
  if (x == y) {
    return t_;
  }

  /* compare the insides if there are any interesting insides */
  //printf("eq-x: "); Expression_dump(x, -1); printf("\n");
  //printf("eq-y: "); Expression_dump(y, -1); printf("\n");

  if (x->type == number || y->type == number) { // TODO - remove
    printf("WARNING: eq between obsolete type\n");
  } 

  if (x->type >= number && y->type >= number) { // TODO - clean
    if (x->type == integer && y->type == integer) { 
      //printf("eq integer: %d %d\n", (*(int*)x->car), (*(int*)y->car)); 
      return (*((int*)x->car) == *((int*)y->car)) ? t_ : nil_;  
    } else if (x->type == real || y->type == real) { 
      //printf("eq real: %f %f\n", to_double(x), to_double(y); 
      return to_double(x) == to_double(y) ? t_ : nil_;
    } 
  }

  if (x->type == string && y->type == string) { 
    printf("eq str: |%s| |%s|\n", x->car, y->car); 
    return (strcasecmp((char*)x->car, (char*)y->car) == 0) ? t_ : nil_;
  }

  return nil_;
}


/**
 * 4. car
 *
 * (car x) expects the value of x to be a list, and returns the first element.
 *
 *   > (car '(a b c))
 *   a
 */
Expression* Expression_car(Expression* expression)
{
  // TODO - logic for null / nil_ / end of list
  return expression->car;
}


/**
 * 5. cdr
 *
 * (cdr x) expects the value of x to be a list, and returns everything after 
 * the first element.
 *
 *   > (cdr '(a b c))
 *   (b c)
 */
Expression* Expression_cdr(Expression* expression)
{
  // TODO - logic for null / nil_ / end of list
  return expression->cdr;
}


/**
 * 6. cons
 *
 * (cons x y) expects the value of y to be a list and returns a list containing the 
 * value of x followed by the elements of the value of y.
 *
 *   > (cons 'a '(b c))
 *   (a b c)
 *   > (cons 'a (cons 'b (cons 'c '())))
 *   (a b c)
 *   > (car (cons 'a '(b c)))
 *   a
 *   > (cdr (cons 'a '(b c)))
 *   (b c) 
 */
Expression* Expression_cons(Expression* x, Expression* y)
{
  if (x == null) { printf("WARNING: cons-x null\n"); }
  if (y == null) { printf("WARNING: cons-y null\n"); }

  _gc_protect(x); 
  _gc_protect(y);
  Expression* cons = _new_Expression(list);
  _gc_unprotect(y);
  _gc_unprotect(x);
  cons->car = x;
  cons->cdr = y;
  
  return cons;
}


/**
 * 7. cond
 *
 * (cond (p_1 e_1) ... (p_n e_n) is evaluated as follows: The p expressions are
 * evaluated in order until one returns t. When one is found, the value of the
 * corresponding e expression is returned as the value of the whole cond
 * expression.
 *
 *   > (cond ((eq 'a 'b) 'first)
 *           ((atom 'a)  'second))
 *   second
 *
 * (defun evcon. (c a)
 *   (cond ((eval. (caar c) a) (eval. (cadar c) a))
 *          ('t (evcon. (cdr c) a))))
 *
 * TODO: add fn to call a lisp function from Expression_eval and replace this
 *       with the above code.
 */
Expression* Expression_cond(Expression* x, Expression* a)
{
  _gc_protect(x); _gc_protect(a);
  Expression* p = Expression_eval(x->car->car, a);
  _gc_protect(p);
  Expression* ret;
  if (p == t_) {
    ret = Expression_eval(x->car->cdr->car, a);
  } else {
    ret = Expression_cond(x->cdr, a);
  }
  _gc_unprotect(p);
  _gc_unprotect(a); _gc_unprotect(x);
  return ret;
}


/**
 * list
 *
 * (list e_1 ... e_n) appends the e expressions to a single list.
 *
 *   > (cons 'a (cons 'b (cons 'c '())))
 *   (a b c)
 *   > (list 'a 'b 'c)
 *   (a b c)
 *
 * TODO - varargs ? deprecate 4 lisp version ?
 */

Expression* Expression_list(Expression* x, Expression* y)
{
  return Expression_cons(x, Expression_cons(y, nil_));
}


/**
 * append
 *
 * (append x y) takes two lists and returns their concatenation.
 *
 * (defun append (x y)
 *   (cond ((null x) y)
 *          ('t (cons (car x) (append (cdr x) y)))))
 *
 *   > (append '(a b) '(c d))
 *   (a b c d)
 *   > (append '() '(c d))
 *   (c d)
 */
Expression* Expression_append(Expression* x, Expression* y)
{
  if (Expression_null(x) == t_) {
    return y;
  } 
  return Expression_cons(x->car, Expression_append(x->cdr, y));
}


/**
 * pair
 *
 * (pair x y) takes two lists of the same length and returns a list of two-
 * element lists containing successive pairs of an element from each.
 *
 * (defun pair (x y)
 *   (cond ((and (null x) (null y)) '())
 *         ((and (not (atom x)) (not (atom y)))
 *          (cons (list (car x) (car y))
 *                (pair (cdr x) (cdr y))))))
 *
 *   > (pair '(x y z) '(a b c))
 *   ((x a) (y b) (z c))
 */
Expression* Expression_pair(Expression* x, Expression* y)
{
  if ((Expression_null(x) == t_) && (Expression_null(y) == t_)) {
    return nil_;
  } else if ((Expression_atom(x) == nil_) && (Expression_atom(y) == nil_)) {
    Expression* l = Expression_list(x->car, y->car);
    Expression* p = Expression_pair(x->cdr, y->cdr);
    Expression* r = Expression_cons(l, p);
    return r;
  }
  printf("pair needs lists not atoms");
  return null;
}



/**
 * assoc 
 *
 * (assoc x y) takes an atom x and a list y of the form created by pair,
 * and  returns the second element of the first list in y whose first
 * element is x.
 *
 * (defun assoc (x y)
 *   (cond ((eq (caar y) x) (cadar y))
 *          ('t (assoc x (cdr y)))))
 *
 *   > (assoc 'x '((x a) (y b)))
 *   a
 *   > (assoc 'x '((x new) (x a) (y b)))
 *   new
 *   > (assoc 'e '((x a) (y b)))
 *   NIL
 *   > (assoc 2 '((x a) (y b)))
 */
/* TODO - rewrite in lisp when the system can handle it */
Expression* Expression_assoc(Expression* x, Expression* y)
{
  // TODO - is this ok ?
  if (x->type != atom) { // || x == t_ || x == nil_) { // TODO <- do we want these in the environment or hard coded ?
    return x;
  }
  if (y != nil_ && y && y->car && y->car->car) {
    if (Expression_eq(y->car->car, x) == t_) {
      return y->car->cdr->car;
    } else if (y->cdr) {
      return Expression_assoc(x, y->cdr);
    }
  }
  //printf("warning: the variable '"); Expression_dump(x,0); printf("' is unbound\n");
  return nil_;
}

/**
 * set
 *
 * (set x y) takes a pair x and substitutes it for a pair in the list of pairs y
 * whose car matches the car of x.
 *
 * Destructive on y
 */
Expression* Expression_set(Expression* x, Expression* y)
{
  //printf("Examining: "); Expression_dump(x->car, 0); printf("...");
  if (Expression_eq(y->car->car, x->car) == t_) {
    //printf("current value is: "); Expression_dump(y->car, 0); printf("\n");
    y->car = x;
    return y;
  } else if (y->cdr) {
    //printf("dig deeper\n");
    return Expression_set(x, y->cdr);
  }

  printf("warning: could not set the variable '"); Expression_dump(x, 0); printf("'\n");
  return y;
}



Expression* Expression_print(Expression* expression)
{
  Expression_dump(expression, -1); 
  printf("\n");
  return expression;
}


/**
 * (defun slow-list-reverse (L)
 *   (if (null L)
 *     nil
 *     (list-append (slow-list-reverse (rest L)) 
 *                  (list (first L)))))
 */
Expression* Expression_reverse(Expression* expression)
{
  if (expression == nil_) {
    return nil_;
  }
  _gc_protect(expression);
  Expression* reversed = Expression_reverse(expression->cdr);
  Expression* consed = Expression_cons(expression->car, nil_);
  _gc_protect(reversed); _gc_protect(consed);
  Expression* ret = Expression_append(reversed, consed);
  _gc_unprotect(consed); _gc_unprotect(reversed); 
  _gc_unprotect(expression);
  return ret;
}


/**
 * eval
 */
Expression* Expression_eval(Expression* e, Expression* a)         /* (defun eval. (e a)                                           */
{                                                                 /*   (cond                                                      */
  //printf("================================================\n");
  //printf("  -> eval-e:"); Expression_dump(e, 0); printf("\n");
  //printf("------------------------------------------------\n");
  //printf("  -> eval-a:"); Expression_dump(a, 0); printf("\n");
  //printf("================================================\n");
  _gc_protect(e); _gc_protect(a);
  Expression* ret = e;

  if (Expression_atom(e) == t_) {                                 /*     ((atom e) (assoc. e a))                                  */
    ret = Expression_assoc(e, a);
    // TODO - weirdness here, lisp in small pieces defines as follows and that helps problems with label:
    //Expression* p = Expression_assoc(e, a);
    //if (p == nil_) { return e; } else { return p; }

  } else if (Expression_atom(e->car) == t_) {                     /*     ((atom (car e))                                          */
                                                                  /*       (cond                                                  */ 
    if (Expression_eq(e->car, quote_) == t_) {                    /*         ((eq (car e) 'quote) (cadr e))                       */        
      ret = Expression_quote(e->cdr->car); 
    } else if (Expression_eq(e->car, atom_) == t_) {              /*         ((eq (car e) 'atom)  (atom   (eval. (cadr e) a)))    */
      ret = Expression_atom(Expression_eval(e->cdr->car, a));
    } else if (Expression_eq(e->car, eq_) == t_) {                /*         ((eq (car e) 'eq)    (eq     (eval. (cadr e) a)      */
      ret = Expression_eq(Expression_eval(e->cdr->car, a),       /*                                      (eval. (caddr e) a)))   */
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, car_) == t_) {               /*         ((eq (car e) 'car)   (car    (eval. (cadr e) a)))    */
      ret = Expression_car(Expression_eval(e->cdr->car, a));
    } else if (Expression_eq(e->car, cdr_) == t_) {               /*         ((eq (car e) 'cdr)   (cdr    (eval. (cadr e) a)))    */
      ret = Expression_cdr(Expression_eval(e->cdr->car, a));
    } else if (Expression_eq(e->car, cons_) == t_) {              /*         ((eq (car e) 'cons)  (cons   (eval. (cadr e) a)      */ 
      ret = Expression_cons(Expression_eval(e->cdr->car, a),     /*                                      (eval. (caddr e) a)))   */
                             Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, cond_) == t_) {              /*         ((eq (car e) 'cond)  (evcon. (cdr e) a))             */
      ret = Expression_cond(e->cdr, a);                          

    /* environment & binding */
    } else if (Expression_eq(e->car, set_) == t_) {  
      // look for symbol in environment, replace if there else append
      Expression* x = Expression_eval(e->cdr->car, a);
      Expression* y = Expression_eval(e->cdr->cdr->car, a);
      Expression* b = Expression_assoc(x, _global_interpreter->environment);
      if (b != nil_) {
        Expression_set(Expression_list(x, y), _global_interpreter->environment);
      } else {
        _global_interpreter->environment = Expression_cons(Expression_list(x, y), _global_interpreter->environment);
      }
      ret = y; // _environment->car->car; 

    /* utilities */
    } else if (Expression_eq(e->car, progn_) == t_) {  
      Expression* eval;
      Expression* loop = e->cdr;
      while (loop != nil_) {
        eval = Expression_eval(loop->car, a);
        loop = loop->cdr;
      }
      ret = eval;

    } else if (Expression_eq(e->car, print_) == t_) {
      ret = Expression_print(Expression_eval(e->cdr->car, a));
    } else if (Expression_eq(e->car, reverse_) == t_) {
      Expression* x = Expression_eval(e->cdr->car, a);
      _gc_protect(x);
      ret = Expression_reverse(x);
      _gc_unprotect(x);
    } else if (Expression_eq(e->car, quit_) == t_) {
      ret = null;

    /* arithmetic */
    } else if (Expression_eq(e->car, gt_) == t_) {             
      ret = Expression_gt(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, lt_) == t_) {             
      ret = Expression_lt(Expression_eval(e->cdr->car, a),
                          Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, gte_) == t_) {             
      ret = Expression_gte(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, lte_) == t_) {             
      ret = Expression_lte(Expression_eval(e->cdr->car, a),
                          Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, add_) == t_) {             
      ret = Expression_add(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, sub_) == t_) {             
      ret = Expression_sub(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, mul_) == t_) {             
      ret = Expression_mul(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));
    } else if (Expression_eq(e->car, div_) == t_) {             
      ret = Expression_div(Expression_eval(e->cdr->car, a),
                           Expression_eval(e->cdr->cdr->car, a));

    /* extensions */
    } else if (Expression_eq(e->car, xl_gc_) == t_) {
      _global_gc->free_list = _gc_collect(); 
      ret = t_;
    } else if (Expression_eq(e->car, xl_environment_) == t_) {
      //ret = _global_interpreter->environment;
      printf("dump environment\n");
      ret = a;
    } else if (Expression_eq(e->car, xl_symbol_list_) == t_) {
      printf("dump symbol-list\n");
      ret = _global_interpreter->symbol_list;

    /* ffi */
    } else if (Expression_eq(e->car, xl_dlopen_) == t_) {
      ret = Expression_dlopen(Expression_eval(e->cdr->car, a));

    } else if (Expression_eq(e->car, xl_dlclose_) == t_) {
      ret = Expression_dlclose(Expression_eval(e->cdr->car, a));

    } else if (Expression_eq(e->car, xl_dlsym_) == t_) {
      ret = Expression_dlsym(Expression_eval(e->cdr->car, a),
                             Expression_eval(e->cdr->cdr->car, a));

    } else if (e->car->type == foreign) {
      /* 1. align, evaluate and fix order of arguments - TODO GC */
      Expression* args = Expression_reverse(e->cdr); 
      Expression* eval_args = args;
      int num_args = 0;
      _gc_protect(args);  
      while (eval_args != nil_) {
        _gc_protect(eval_args);
        eval_args->car = Expression_eval(eval_args->car, a);
        _gc_unprotect(eval_args);
        //printf("  Arg: %d ", eval_args->car->type); Expression_dump(eval_args->car, -1); printf("\n");
        if (eval_args->car->type == real) {
          num_args++;
        } else {
          num_args++;
        }
        eval_args = eval_args->cdr;
      }
      _gc_unprotect(args);
      size_t mod_args = (num_args % 4);
      for (size_t t = 0; mod_args != 0 && t != (4 - mod_args); t++) {
        Expression* n = _new_Expression(integer);
        int* p = malloc(sizeof(int));
        *p = 0;
        n->car = (void*)p;
        _gc_protect(n);  
        args = Expression_cons(n, args);
        _gc_unprotect(n);
      }

      _gc_protect(args);  
      Expression* invocation = Expression_cons(e->car, args);
      _gc_unprotect(args);

      /* 2. Call foreign function */
      _gc_protect(invocation);
      ret = Expression_call(invocation);
      _gc_unprotect(invocation);



    } else {                                                      /*         ('t (eval. (cons (assoc. (car e) a) (cdr e)) a))))   */
      Expression* p = Expression_cons(Expression_assoc(e->car, a), e->cdr);
      _gc_protect(p);
      ret = Expression_eval(p, a);        
      _gc_unprotect(p);
    }
  } else if (Expression_eq(e->car->car, label_) == t_) {          /*     ((eq (caar e) 'label)                                    */
    Expression* x = Expression_cons(e->car->cdr->cdr->car,e->cdr);/*       (eval. (cons (caddar e) (cdr e))                       */
    Expression* y = Expression_cons(Expression_list(e->car->cdr->car, /*          (cons (list. (cadar e) (car e)) a)))            */
                                                    e->car),  a);                        
    _gc_protect(x); _gc_protect(y);
    ret = Expression_eval(x, y);
    _gc_unprotect(y); _gc_unprotect(x);
  } else if (Expression_eq(e->car->car, lambda_) == t_) {         /*     ((eq (caar e) 'lambda)                                   */
                                                                  /*       (eval. (caddar e)                                      */
                                                                  /*              (append. (pair. (cadar e)                       */
                                                                  /*                              (evlis. (cdr e) a))             */
                                                                  /*                       a)))))                                 */
    Expression* evlis = Expression_evlis(e->cdr, a);
    Expression* pair = Expression_pair(e->car->cdr->car, Expression_evlis(e->cdr, a));
    Expression* append = Expression_append(pair, a);
    //printf("\ne:\t>>"); Expression_dump(e->car->cdr->cdr->car, -1); printf ("<<\n");
    //printf("a:\t>>"); Expression_dump(append, -1); printf ("<<\n");
    //Expression* eval = Expression_eval(e->car->cdr->cdr->car, append);
    //printf("eval:\t>>"); Expression_dump(eval, -1); printf ("<<\n");
    ret = Expression_lambda(e->car->cdr->cdr, append);
  } 

  /*if (ret == e) {
    printf("\nFELLTHROUGH\n");
   }*/
  _gc_unprotect(a); _gc_unprotect(e);
  return ret;
}



Expression* Expression_lambda(Expression* l, Expression* a)
{
  Expression* eval;
  _gc_protect(l); _gc_protect(a);
  Expression* loop = l;
  while (loop != nil_) {
    eval = Expression_eval(loop->car, a);
    //printf("eval:\t>>"); Expression_dump(eval, -1); printf ("<<\n");
    loop = loop->cdr;
  }
  _gc_unprotect(a); _gc_unprotect(l);
  return eval;
}

/**
 * lambda
 *
 *   > ((lambda (x) (cons x '(b))) 'a)
 *   (a b)
 *   > ((lambda (x y) (cons y (cons x '()))) 'a 'b)  
 *   (b a)
 *   > ((lambda (x) (add x 1)) 2)
 *   3
 *   > ((lambda (f) (f '(b c))) '(lambda (x) (cons 'a x)))
 *   (a b c)
 */



/**
 * label
 */


/**
 * evlis
 *
 * (defun evlis. (m a)
 *   (cond ((null. m) '())
 *         ('t (cons (eval.  (car m) a)
 *                   (evlis. (cdr m) a)))))
 *
 * TODO rewrite in lisp
 */
Expression* Expression_evlis(Expression* m, Expression* a) 
{
  if (Expression_null(m) == t_) {
    return nil_;
  } 
  _gc_protect(m); _gc_protect(a);
  Expression* eval = Expression_eval(m->car, a);
  Expression* evlis = Expression_evlis(m->cdr, a);
  _gc_protect(eval); _gc_protect(evlis);
  Expression* ret = Expression_cons(eval, evlis);
  _gc_unprotect(evlis); _gc_unprotect(eval);
  _gc_unprotect(a); _gc_unprotect(m);
  return ret;
}



/*
(defun eval. (e a)
  (cond
    ((atom e) (assoc. e a))
    ((atom (car e))
     (cond
       ((eq (car e) 'quote) (cadr e))
       ((eq (car e) 'atom)  (atom   (eval. (cadr e) a)))
       ((eq (car e) 'eq)    (eq     (eval. (cadr e) a)
                                    (eval. (caddr e) a)))
       ((eq (car e) 'car)   (car    (eval. (cadr e) a)))
       ((eq (car e) 'cdr)   (cdr    (eval. (cadr e) a)))
       ((eq (car e) 'cons)  (cons   (eval. (cadr e) a)
                                    (eval. (caddr e) a)))
       ((eq (car e) 'cond)  (evcon. (cdr e) a))
       ('t (eval. (cons (assoc. (car e) a)
                        (cdr e))
                  a))))
    ((eq (caar e) 'label)
     (eval. (cons (caddar e) (cdr e))
            (cons (list. (cadar e) (car e)) a)))
    ((eq (caar e) 'lambda)
     (eval. (caddar e)
            (append. (pair. (cadar e) (evlis. (cdr e) a))
                     a)))))
 */


/*
;; (defun add (eax ebx) '(emit + eax ebx))                                      
(defun add (eax ebx) (+ eax ebx))
(defun plus (terms)
  (if (eq terms nil)
      0
    (+ (car terms) (plus (cdr terms)))
    ))
(defun vplus (&rest terms)
  (plus terms))
(vplus 1 2 3 4)
 */


 /*
TODO - the varargs here is causing major shit when called from Expression_pair - check on linux
Expression* Expression_list(Expression* e_1, ...)
{
  Expression* list = Expression_cons(e_1, nil_);
  Expression* e_n = null;
  size_t t = 0;
  printf("\n  ->%d. %s\n", t++, e_1->car);
  va_list argp;
  va_start(argp, e_1);
  while ((e_n = va_arg(argp, Expression*)) != null) {
    printf("%d (%u)\n", t, e_n);
    printf("  ->%d. %s\n", t++, (e_n->car ? e_n->car : "e_n-null"));
    list = Expression_append(list, Expression_cons(e_n, nil_));
  }
  va_end(argp);
  return list;
}*/
