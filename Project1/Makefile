CC=g++
TARGET=init

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR) -DD_REENTRANT -Wall -Werror
OBJCFLAGS=-O3 -c -I$(INCDIR) -DD_REENTRANT
LFLAGS=-lrt -lpthread

DEPS=clock_device.o char_device.o block_device.o id.o cpu.o scheduler.o fcfs.o round_robin.o\
		kernel.o abstract_device.o logger.o process_logger.o #lottery.o
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


$(TARGET): $(TARGET).cpp $(TARGET).h $(DEPS)
	$(CC) $(CFLAGS) $(LFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@ 

%.o: %.cpp %.h
	$(CC) $< $(OBJCFLAGS) -o $(OBJDIR)/$@
	
run: all
	cd $(BINDIR) && ./$(TARGET)
	
run_fcfs: all
	cd $(BINDIR) && ./$(TARGET) -scheduler fcfs

run_rr: all
	cd $(BINDIR) && ./$(TARGET) -scheduler round-robin
	
run_lottery: all
	cd $(BINDIR) && ./$(TARGET) -scheduler lottery
	
gdb_%: all
	gdb --cd=$(BINDIR) --command=gdb_batch_$* --return-child-result -exec=$(TARGET)
	
run_top: top
	./$(BINDIR)/$^ $(BINDIR)
	
top: top.cpp top.h
	$(CC) -Iinclude $(SRCDIR)/top.cpp -o $(BINDIR)/$@
	
test_sock: test_name_socket.cpp
	$(CC) -Iinclude $^ -o $(BINDIR)/$@
	
run_sock_test: test_sock
	cd $(BINDIR) && ./$^ 0
	
valgrind_%: all
	cd $(BINDIR) && valgrind --tool=memcheck --track-origins=yes --leak-check=full ./$(TARGET) -scheduler $*
	
update_docs:
	doxygen Doxyfile
	
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(TARGET)
	
clean_docs:
	rm -rf $(DOCDIR)/Doxygen/*
	
clean_all: clean clean_docs
