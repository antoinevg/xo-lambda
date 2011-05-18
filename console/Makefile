APP = gtk_console
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
INCLUDES = -I. 
SWIG = swig

# configure shared library command for platform
UNAME = $(shell uname)
ifeq ($(UNAME),Linux)
	LDSHARED += -shared -lcairo -lfreetype 
endif
ifeq ($(UNAME),Darwin)
	LDSHARED += -dylib -flat_namespace -undefined suppress
endif

PHLO_CFLAGS = -I../phlo
LITHP_CFLAGS = -I../lithp
LITHP_LDFLAGS = ../lithp/liblithp.a
CFLAGS += -std=c99 -g -Wall
CFLAGS += $(INCLUDES)
CFLAGS += $(PHLO_CFLAGS) 
CFLAGS += $(LITHP_CFLAGS) 
CFLAGS += `pkg-config gtk+-2.0 --cflags`
LDFLAGS += $(LITHP_LDFLAGS)
LDFLAGS += `pkg-config gtk+-2.0 --libs`
BINDINGS_CFLAGS += $(CFLAGS) `python-config --cflags`
BINDINGS_LDFLAGS += $(LITHP_LDFLAGS) 
BINDINGS_LDFLAGS += `python-config --libs`


gtk_console: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)  
	ln -s ext/Monaco_Linux.ttf .


python-bindings: check-platform console.i 
	$(SWIG) -python console.i
	$(CC) $(BINDINGS_CFLAGS) -c console_wrap.c
	$(LD) $(LDSHARED) -o _console.so console.o console_wrap.o $(BINDINGS_LDFLAGS)


all: $(APP) python-bindings


check-platform: 
	@echo Checking platform...$(UNAME)

clean:
	$(RM) -r $(APP) *.o *~ $(APP).dSYM console.py console_wrap.c *.so *.pyc Monaco_Linux.ttf

