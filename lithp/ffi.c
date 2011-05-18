/* ffi.c -- foreign function interface
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

#include <dlfcn.h>


void test0()                    { printf("test0()\n"); }
void test1(char* a)                    { printf("test1(%s)\n", a); }
void test2(char* a, char* b)           { printf("test2(%s, %s)\n",a ,b); }
void test3(int a, char* b, int c)      { printf("test3(%d, %s, %d)\n", a, b, c); }
void test3s(char* a, char* b, char* c) { printf("test3s(%s, %s, %s)\n",a ,b, c); }

//#define TEST_FFI
#ifdef TEST_FFI
int main(int argc, char** argv)
{
  printf("Testing ffi...\n");

  void* global_handle = dlopen(null, RTLD_LAZY);
  if (global_handle == null) {
    perror("Could not open self as a dynamic library");
    perror(dlerror());
    return EXIT_FAILURE;
  }
  /* Gad - this is probably unimaginably evil */
  unsigned int (*fn_test)() = dlsym(global_handle, "test0");
  if (fn_test == null) {
    perror(dlerror());
  }
  fn_test();
  fn_test = dlsym(global_handle, "test1"); fn_test("one");
  fn_test = dlsym(global_handle, "test2"); fn_test("one", "two");
  fn_test = dlsym(global_handle, "test3"); fn_test(1, "two", 3);

  /* printf("Call directly\n");
  __asm__ __volatile__ (
    "push %%esi\n"
    "push %%edi\n"
    "push %%eax\n"
    "call %%ecx\n"
    :
    : "S" (args[2]),
      "D" (args[1]),
      "A" (args[0]),
      "c" (fn_test)
  );*/

  void* handle = dlopen("libc.dylib", RTLD_LAZY); /* RTLD_NOW | RTLD_GLOBAL | RTLD_LOCAL */
  if (handle == null) {
    perror("Could not open dynamic library");
    perror(dlerror());
    return EXIT_FAILURE;
  }
  printf("\n");


  /* Impossibly evil - TODO rewrite this whole thing as assembler */
  printf(">>>>");
  unsigned int (*fn)();
  char* args[] = { "hello", "foo plink %s" }; //, "bar", "plonk" };
  size_t len = sizeof(args) / sizeof(char*);
  fn = dlsym(handle, "printf"); 
  /* 1. push function arguments onto stack */
  for (size_t t = 0; t < len; t++) {
    __asm__ __volatile__ ("push %%esi" : : "S" (args[t]));     
  }
  /* 2. call function */
  //__asm__ __volatile__ ("call %0" : : "m" (fn));                    
  unsigned int ret = fn();   
  /* 3. clean the stack */
  for (size_t t = 0; t < len; t++) {
    __asm__ __volatile__ ("popl %esi");
  }
  printf("<<<<\n");

  printf("\n");
  //void* (*f)();
  float (*f)();
  void* symbol_address = dlsym(handle, "printf");
  if (symbol_address == null) {
    perror("Could not locate symbol");
    perror(dlerror());
  }
  f = symbol_address;
  printf("|%d|\n", f("Boo %d!\n", 42));
  f("Hello there!\n");

  f = dlsym(handle, "sqrt");
  if (f == null) { perror(dlerror()); }
  float ff = f(9.0f);
  printf("sqrt: %f\n", ff);

  if (dlclose(handle) == -1) {
    perror("Could not close dynamic library");
    perror(dlerror());
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
#endif /* TEST_FFI */



/**
 * dlopen
 */
Expression* Expression_dlopen(Expression* path)
{
  if (path == null || path->type != string) {
    printf("WARNING: Expression_dlopen() - Invalid path\n");
    return nil_;
  }

  void* handle = dlopen((char*)path->car, RTLD_LAZY); /* RTLD_NOW | RTLD_GLOBAL | RTLD_LOCAL */
  if (handle == null) {
    perror(dlerror());
    return nil_;
  }

  Expression* ret = _new_Expression(pointer);
  ret->car = handle;
  return ret;
}



/**
 * dlclose
 */
Expression* Expression_dlclose(Expression* handle)
{
  if (handle == null || handle->type != pointer) {
    printf("WARNING: Expression_dlclose() - Invalid handle\n");
    return nil_;
  }

  if (dlclose(handle->car) == -1) {
    perror(dlerror());
    return nil_;
  }

  return t_;
}



/**
 * dlsym
 */
Expression* Expression_dlsym(Expression* handle, Expression* symbol)
{
  if (handle == null || handle->type != pointer || symbol == null || symbol->type != string) {
    printf("WARNING: Expression_dlsym() - Invalid arguments\n");
    return nil_;
  }

  void* symbol_address = dlsym(handle->car, (void*)symbol->car);
  if (symbol_address == null) {
    perror(dlerror());
    return nil_;
  }
  
  //unsigned int (*f)() = symbol_address;
  //f("Expression_dlsym testing got %p\n", symbol_address);

  _gc_protect(handle); _gc_protect(symbol);
  Expression* ret = _new_Expression(foreign);
  ret->car = symbol_address;
  _gc_unprotect(symbol); _gc_unprotect(handle);

  return ret;
}



/**
 * call
 *
 * Unspeakably filthy - do _not_ try this at home kids.  
 *
 * TODO - clean it
 */
Expression* Expression_call(Expression* e)
{
  printf("Calling: "); Expression_dump(e, -1); printf("\n");

  /* 1. Setup function pointer - unsigned int vs void**/
  void* result = 0;
  void* (*f)() = (void*)e->car->car;

  /* save registers and stack pointer */
  /*
  __asm__ __volatile__ ("push %ebp\n"
                        "mov %esp, %ebp"
                        );*/
  //__asm__ __volatile__ ("pushl %edi");

  /* 2. push function arguments onto stack */
  Expression* args = e->cdr;
  while (args != nil_) {
    if (args->car->type == integer) {
      __asm__ __volatile__ ("pushl %0" : : "nr" ((unsigned int) (*(int*)args->car->car)));     
    } else if (args->car->type == real) {
      __asm__ __volatile__ (
        "fldl  %0\n"               /* load floating point value onto the FPU register stack */
        "subl  $8, %%esp\n"        /* decrement stack pointer */
        "fstpl (%%esp)\n"          /* store floating point value to location pointed to by stack pointer */
        : : "m" (*(double*)args->car->car)); /* we could use 't' here and lose fldl if we want, but 4 now I want things explicit */
    } else if (args->car->type == pointer) {
      __asm__ __volatile__ ("pushl %0" : : "r" (args->car->car));     
    } else if (args->car->type == string) {
      __asm__ __volatile__ ("pushl %0" : : "r" (args->car->car));     
    } else if (args->car == nil_) {
      __asm__ __volatile__ ("pushl %0" : : "r" (null));     
    } else if (args->car == t_) {
      __asm__ __volatile__ ("pushl %0" : : "nr" ((char)1));      // TODO - check
    }
    args = args->cdr;
  }

  /* 3. call function */
  __asm__ __volatile__ (
    "call %1       \n"
    "mov %%eax, %0 \n" 
    : "=r" (result) 
    : "m" (f));
  //result = f();

  /* 5. clear the stack */
  args = e->cdr;
  while (args != nil_) {
    if (args->car->type == real) {
      //__asm__ __volatile__ ("addl $8, %esp");
      __asm__ __volatile__ ("popl %esi");
    } else {
      __asm__ __volatile__ ("popl %esi");
    }
    args = args->cdr;
  }

  /* restore registers and stack pointer */
  
  //__asm__ __volatile__ ("popl %edi\n");
  /*__asm__ __volatile__ (//"mov %esp, %ebp\n"
                        "pop %ebp\n"
                        );
  */



  /* 6. return result - TODO we're really starting to feel the need for numeric types */
  //double* d = malloc(sizeof(double)); // convert to double
  //*d = result;
  Expression* ret = _new_Expression(pointer);
  ret->car = (void*)result; //d;

  return ret;  
}
