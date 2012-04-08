FLAGS = -g -c -Wwrite-strings -I.
AFLAGS = -g -I.

BINS = Kernel.o BitBlock.o IndexNode.o Block.o DirectoryEntry.o  ProcessContext.o FileDescriptor.o  Stat.o FileSystem.o SuperBlock.o RandomAccessFile.o io_types.o

APPS = mkfs cat tee cp ls ln rm mkdir dump df

all:
	make $(BINS)

all_apps:
	make $(APPS)

clean:
	\rm -rf $(BINS)
	
clean_all: clean
	rm -rf $(APPS)

cat: cat.cc $(BINS)
	g++ $(AFLAGS) cat.cc $(BINS) -o cat

dump: dump.cc $(BINS)
	g++ $(AFLAGS) dump.cc $(BINS) -o dump
	
df: df.cc $(BINS)
	g++ $(AFLAGS) df.cc $(BINS) -o df

cp: cp.cc $(BINS)
	g++ $(AFLAGS) cp.cc $(BINS) -o cp

ls: ls.cc $(BINS)
	g++ $(AFLAGS) ls.cc $(BINS) -o ls
	
ln: ln.cc $(BINS)
	g++ $(AFLAGS) ln.cc $(BINS) -o ln
	
rm: rm.cc $(BINS)
	g++ $(AFLAGS) rm.cc $(BINS) -o rm

tee: tee.cc $(BINS)
	g++ $(AFLAGS) tee.cc $(BINS) -o tee

mkfs: mkfs.cc $(BINS)
	g++ $(AFLAGS) mkfs.cc $(BINS) -o mkfs

mkdir: mkdir.cc $(BINS)
	g++ $(AFLAGS) mkdir.cc $(BINS) -o mkdir

io_types.o: io_types.h io_types.cc
	g++ $(FLAGS) io_types.cc

IndexNode.o: IndexNode.h IndexNode.cc
	g++ $(FLAGS) IndexNode.cc

RandomAccessFile.o: RandomAccessFile.h RandomAccessFile.cc
	g++ $(FLAGS) RandomAccessFile.cc

BitBlock.o: BitBlock.h BitBlock.cc
	g++ $(FLAGS) BitBlock.cc

Block.o: Block.h Block.cc
	g++ $(FLAGS) Block.cc

Kernel.o: Kernel.h Kernel.cc
	g++ $(FLAGS) Kernel.cc

DirectoryEntry.o: DirectoryEntry.h DirectoryEntry.cc
	g++ $(FLAGS) DirectoryEntry.cc

ProcessContext.o: ProcessContext.h ProcessContext.cc
	g++ $(FLAGS) ProcessContext.cc

FileDescriptor.o: FileDescriptor.h FileDescriptor.cc
	g++ $(FLAGS) FileDescriptor.cc

Stat.o: Stat.h Stat.cc
	g++ $(FLAGS) Stat.cc


FileSystem.o: FileSystem.h FileSystem.cc
	g++ $(FLAGS) FileSystem.cc

SuperBlock.o: SuperBlock.h SuperBlock.cc
	g++ $(FLAGS) SuperBlock.cc





