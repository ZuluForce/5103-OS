CC=g++
TARGET=init

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR) -DD_REENTRANT
OBJCFLAGS=-O3 -c -I$(INCDIR) -DD_REENTRANT
LFLAGS=-lrt -lpthread

DEPS=clock_device.o char_device.o block_device.o id.o cpu.o scheduler.o fcfs.o\
		kernel.o abstract_device.o
##Dependencies with obj directory appended
_DEPS=$(patsubst %,$(OBJDIR)/%,$(DEPS))

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin
DOCDIR=Documentation

vpath %.cpp $(SRCDIR)
vpath %.cpp $(SRCDIR)/devices
vpath %.cpp $(SRCDIR)/utility
vpath %.cpp $(SRCDIR)/scheduler

vpath %.h $(INCDIR)
vpath %.h $(INCDIR)/devices
vpath %.h $(INCDIR)/utility
vpath %.h $(INCDIR)/scheduler

vpath %.o $(OBJDIR)

all: $(TARGET)


$(TARGET): init.cpp init.h $(DEPS)
	$(CC) $(CFLAGS) $(LFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@ 

%.o: %.cpp %.h
	$(CC) $< $(OBJCFLAGS) -o $(OBJDIR)/$@
	
run: all
	cd $(BINDIR) && ./init
	
valgrind: all
	cd $(BINDIR) && valgrind --tool=memcheck --track-origins=yes --leak-check=full ./$(TARGET)
	
update_docs:
	doxygen Doxyfile
	
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/init
	
clean_docs:
	rm -rf $(DOCDIR)/Doxygen/*
	
clean_all: clean clean_docs
