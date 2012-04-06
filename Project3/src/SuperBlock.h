/* SuperBlock: Abstraction for file system super block */

#include <io_types.h>
#include <RandomAccessFile.h>

class SuperBlock {
private:
	static short blockSize ;
	int blocks ; // total blocks in the system

	// Offset in blocks of the free list block region from the beginning
	// of the file system.
	int freeListBlockOffset ;

	// Offset in blocks of the inode block region from the beginning
	// of the file system.
	int inodeBlockOffset ;

	// Offset in blocks of the data block region from the beginning
	// of the file system.
	int dataBlockOffset ;

public:
	SuperBlock();
	void setBlockSize(short );
	static short getBlockSize();
	void setBlocks(int);
	int getBlocks();
	void setFreeListBlockOffset(int );
	int getFreeListBlockOffset();
	void setInodeBlockOffset(int );
	int getInodeBlockOffset();
	void setDataBlockOffset(int );
	int getDataBlockOffset();
	void write(RandomAccessFile*) ;
	void read(RandomAccessFile* );
};
