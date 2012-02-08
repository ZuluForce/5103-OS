CC=g++

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR)
OBJCFLAGS=-O3 -c -I$(INCDIR)
##LFLAGS=

DEPS=clock_device.o char_device.o block_device.o
##Dependencies with obj directory appended
_DEPS=$(patsubst %,$(OBJDIR)/%,$(DEPS))

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

vpath %.cpp $(SRCDIR)
vpath %.cpp $(SRCDIR)/devices

vpath %.h $(INCDIR)
vpath %.h $(INCDIR)/devices

vpath %.o $(OBJDIR)

all: init


init: init.cpp init.h $(DEPS)
	$(CC) $(CFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@ 

%.o: %.cpp %.h
	$(CC) $< $(OBJCFLAGS) -o $(OBJDIR)/$@
	
	
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/init
