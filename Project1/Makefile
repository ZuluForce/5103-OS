CC=g++
CFLAGS=-O3
OBJCFLAGS=-O3
##LFLAGS=

DEPS=clock_device.o char_device.o block_device.o

SRCDIR=src
OBJDIR=obj
vpath %.cpp $(SRCDIR)
vpath %.h $(INCDIR)

##all:


%.o: %.cpp %.h
	$(CC) $^ -o $(OBJDIR)/$@ $(OBJCFLAGS)
