

#include "lithp.h"


double to_double(Expression* n) {
  if (n->type == integer) {
    //printf("converting %d\n", *((int*)n->car));
    return (double) *((int*)n->car);
  } else if (n->type == real) {
    //printf("converting %f\n", *((double*)n->car));
    return *((double*)n->car);
  } else { 
    printf("NaN - %d %d\n", n->type, *((int*)n->car));
    return 0;
  }
}

/**
 * Arithmetic
 *
 * TODO - support for natural numbers vs integers and rationals
 */
Expression* Expression_gt(Expression* x, Expression* y)
{
  if (x == null || y == null || x->type <= number || y->type <= number) {
    printf ("Error: gt - invalid arguments"); return nil_;
  }
  if (x->type == integer && y->type == integer) {
    return (*((int*)x->car) > *((int*)y->car)) ? t_ : nil_;
  } else if (x->type == real ||  y->type == real) {
    return (to_double(x) > to_double(y)) ? t_ : nil_;
  }
}

Expression* Expression_lt(Expression* x, Expression* y)
{
  if (x == null || y == null || x->type <= number || y->type <= number) {
    printf ("Error: lt - invalid arguments"); return nil_;
  }
  if (x->type == integer && y->type == integer) {
    return (*((int*)x->car) < *((int*)y->car)) ? t_ : nil_;
  } else if (x->type == real ||  y->type == real) {
    return (to_double(x) < to_double(y)) ? t_ : nil_;
  }
}

Expression* Expression_gte(Expression* x, Expression* y)
{
  if (x == null || y == null || x->type <= number || y->type <= number) {
    printf ("Error: gte - invalid arguments"); return nil_;
  }
  if (x->type == integer && y->type == integer) {
    return (*((int*)x->car) >= *((int*)y->car)) ? t_ : nil_;
  } else if (x->type == real ||  y->type == real) {
    return (to_double(x) >= to_double(y)) ? t_ : nil_;
  }
}

Expression* Expression_lte(Expression* x, Expression* y)
{
  if (x == null || y == null || x->type <= number || y->type <= number) {
    printf ("Error: lte - invalid arguments"); return nil_;
  }
  if (x->type == integer && y->type == integer) {
    return (*((int*)x->car) <= *((int*)y->car)) ? t_ : nil_;
  } else if (x->type == real ||  y->type == real) {
    return (to_double(x) <= to_double(y)) ? t_ : nil_;
  }
}

Expression* Expression_add(Expression* x, Expression* y)
{
  ExpressionType type = (x->type == real || y->type == real) ? real : integer; 
  void* p;
  if (type == integer) {
    int* result = malloc(sizeof(int));
    *result = *((int*)x->car) + *((int*)y->car);
    p = (void*)result;
  } else if (type == real) {
    double* result = malloc(sizeof(double));
    *result = to_double(x) + to_double(y);
    p = (void*)result;
  }
  _gc_protect(x); _gc_protect(y);
  Expression* z = _new_Expression(type);
  _gc_unprotect(y); _gc_unprotect(x);
  z->car = p;
  return z;
}

Expression* Expression_sub(Expression* x, Expression* y)
{
  ExpressionType type = (x->type == real || y->type == real) ? real : integer; 
  void* p;
  if (type == integer) {
    int* result = malloc(sizeof(int));
    *result = *((int*)x->car) - *((int*)y->car);
    p = (void*)result;
  } else if (type == real) {
    double* result = malloc(sizeof(double));
    *result = to_double(x) - to_double(y);
    p = (void*)result;
  }
  _gc_protect(x); _gc_protect(y);
  Expression* z = _new_Expression(type);
  _gc_unprotect(y); _gc_unprotect(x);
  z->car = p;
  return z;
}

Expression* Expression_mul(Expression* x, Expression* y)
{
  ExpressionType type = (x->type == real || y->type == real) ? real : integer; 
  void* p;
  if (type == integer) {
    int* result = malloc(sizeof(int));
    *result = *((int*)x->car) * *((int*)y->car);
    p = (void*)result;
  } else if (type == real) {
    double* result = malloc(sizeof(double));
    *result = to_double(x) * to_double(y);
    p = (void*)result;
  }
  _gc_protect(x); _gc_protect(y);
  Expression* z = _new_Expression(type);
  _gc_unprotect(y); _gc_unprotect(x);
  z->car = p;
  return z;
}

Expression* Expression_div(Expression* x, Expression* y)
{
  ExpressionType type = (x->type == real || y->type == real) ? real : integer; 
  void* p;
  if (type == integer) {
    int* result = malloc(sizeof(int));
    *result = *((int*)x->car) / *((int*)y->car);
    p = (void*)result;
  } else if (type == real) {
    double* result = malloc(sizeof(double));
    *result = to_double(x) / to_double(y);
    p = (void*)result;
  }
  _gc_protect(x); _gc_protect(y);
  Expression* z = _new_Expression(type);
  _gc_unprotect(y); _gc_unprotect(x);
  z->car = p;
  return z;
}
