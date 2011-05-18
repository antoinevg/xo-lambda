/* http://www.scriptol.org/sieve.php      */
/* Sieve Of Erathosthenes by Denis Sureau */
#include <stdlib.h> 
#include <stdio.h>

void eratosthenes(int top)
{
  int all[1000000];
  int idx = 0;
  int prime = 3;
  int x, j;
  
  printf("1 ");
  
  while (prime <= top) {
    for(x = 0; x < top; x++) {
      if(all[x] == prime) goto skip; 
    }
    
    printf("%d ", prime);
    j = prime;
    while(j <= (top / prime)) {
      all[idx++] = prime * j;
      j += 1;
    }
    
  skip:
    prime+=2;
  }
  puts("");
  return;
}

int main(int argc, char **argv)
{
  if (argc == 2) {
    eratosthenes(atoi(argv[1]));
  } else {
    eratosthenes(1000000);
  }
  return 0;
}
