CC=gcc -std=c99
CFLAGS=-I. -I../phlo -g -masm=att
LD=ld
LDFLAGS=-ldl
LDSHARED = -dylib -lc 
#-flat_namespace -undefined suppress
#LDSHARED = -bundle -flat_namespace -undefined suppress

objects = lithp.o parse.o math.o ffi.o main.o 


bootstrap: $(objects)
	$(CC) $(CFLAGS) -o bootstrap $(objects) $(LDFLAGS)


liblithp: lithp.o parse.o math.o ffi.o 
	$(AR) rcs liblithp.a $^
	# $(LD) $(LDSHARED) -o liblithp.so $^ $(LDFLAGS)


ffi: lithp.o ffi.o
	$(CC) $(CFLAGS) -D TEST_FFI -o ffi $^


lithp.o : lithp.h lithp.c
parse.o : lithp.h parse.c
math.o  : lithp.h math.c
ffi.o   : lithp.h ffi.c
main.o : lithp.h main.c

all: bootstrap liblithp

.PHONY : clean
clean:
	rm -rf *~ tests/*~ tests/*.png tests/test_cairo *.o bootstrap bootstrap.dSYM *.a *.so
