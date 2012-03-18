CC=g++
TARGET=main

RUN_R1=-t main.trace -s settings.ini

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR) -Wall
OBJCFLAGS=-O3 -c -I$(INCDIR)
LFLAGS=-L/usr/lib/

DEPS= vmm_core.o iniReader.o mmu.o cpu.o round_robin.o frameAlloc.o pageReplace.o\
	pr_fifo.o defaults.o id.o strConv.o io_control.o logger.o
##Dependencies with obj directory appended
_DEPS=$(patsubst %,$(OBJDIR)/%,$(DEPS))

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin
DOCDIR=docs

vpath %.cpp $(SRCDIR)
vpath %.cpp $(SRCDIR)/Policy
vpath %.cpp $(SRCDIR)/settings
vpath %.cpp $(SRCDIR)/utility

vpath %.h $(INCDIR)
vpath %.h $(INCDIR)/Policy
vpath %.h $(INCDIR)/settings
vpath %.h $(INCDIR)/utility

vpath %.o $(OBJDIR)

all: $(TARGET)

run_%: $(TARGET)
	cd $(BINDIR) && ./$(TARGET) $(RUN_$*)

$(TARGET): $(TARGET).cpp $(TARGET).h $(DEPS)
	$(CC) $(CFLAGS) $(LFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@
	
run_gdb:
	make CFLAGS="-I$(INCDIR) -Wall -g -DDEBUG" OBJCFLAGS="-DDEBUG -g -c -I$(INCDIR)"
	gdb --cd=$(BINDIR) --command=gdb_batch --return-child-result -exec=$(TARGET)
	
filt: $(TARGET).cpp $(TARGET).h $(DEPS)
	make CC="./scripts/gfilt"

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
