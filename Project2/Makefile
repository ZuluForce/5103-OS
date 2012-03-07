CC=g++
TARGET=init

SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=bin

CFLAGS=-O3 -I$(INCDIR) -DD_REENTRANT -Wall -Werror
OBJCFLAGS=-O3 -c -I$(INCDIR) -DD_REENTRANT
LFLAGS=-lrt -lpthread

DEPS= ##*.o
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


$(TARGET): $(TARGET).cpp $(TARGET).h $(DEPS)
	$(CC) $(CFLAGS) $(LFLAGS) $(_DEPS) $(SRCDIR)/$@.cpp -o $(BINDIR)/$@ 

%.o: %.cpp %.h
	$(CC) $< $(OBJCFLAGS) -o $(OBJDIR)/$@
	
run_%: all
	cd $(BINDIR) && ./$(TARGET) -scheduler $*
	
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