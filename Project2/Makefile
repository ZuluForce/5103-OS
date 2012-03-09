CC=g++
TARGET=main

RUN_R1=-t main.trace -s settings.ini

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR) -Wall
OBJCFLAGS=-O3 -c -I$(INCDIR) -DD_REENTRANT
LFLAGS=-L/usr/lib/

DEPS= vmm_core.o iniReader.o mmu.o cpu.o
##Dependencies with obj directory appended
_DEPS=$(patsubst %,$(OBJDIR)/%,$(DEPS))

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin
DOCDIR=docs

vpath %.cpp $(SRCDIR)

vpath %.h $(INCDIR)

vpath %.o $(OBJDIR)

all: $(TARGET)

run_%: $(TARGET)
	cd $(BINDIR) && ./$(TARGET) $(RUN_$*)

$(TARGET): $(TARGET).cpp $(TARGET).h $(DEPS)
	$(CC) $(CFLAGS) $(LFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@ 

%.o: %.cpp %.h
	$(CC) $< $(OBJCFLAGS) -o $(OBJDIR)/$@
	
update_docs:
	doxygen Doxyfile
	
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(TARGET)
	
clean_docs:
	rm -rf $(DOCDIR)/Doxygen/*
	
clean_all: clean clean_docs
