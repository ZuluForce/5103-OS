/* FileSystem: abstraction of a simulated file system */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <io_types.h>
#include <IndexNode.h>
#include <BitBlock.h>

class FileSystem {
private:
	RandomAccessFile *file;
	String filename;
	String mode;
	short blockSize;
	int blockCount;
	int freeListBlockOffset;
	int inodeBlockOffset;
	int dataBlockOffset;
	IndexNode *rootIndexNode;

public:
	static short const ROOT_INDEX_NODE_NUMBER = 0;
	static int const NOT_A_BLOCK = 0x00FFFFFF;
	FileSystem(String newFilename , String newMode);  // throws IOException
	short getBlockSize();
	int getFreeListBlockOffset();
	int getInodeBlockOffset();
	int getDataBlockOffset();
	IndexNode *getRootIndexNode();

	void open(); // throws IOException
	void close(); // throws IOException
	void read(byte *bytes, int blockNumber); // throws IOException
	void write(byte *bytes , int blockNumber); // throws IOException

private:
	int currentFreeListBitNumber;
	int currentFreeListBlock;
	BitBlock *freeListBitBlock;

public:
	void freeBlock(int dataBlockNumber);  // throws IOException
	int allocateBlock();   // throws IOException

private:
	void loadFreeListBlock(int dataBlockNumber);  // throws IOException

	// The index node number that will next be checked to see
	// if it is available.
	short currentIndexNodeNumber;

	// The number of the index node block which is currently
	// loaded into indexBlockBytes.  If no block is loaded,
	// this contains the value "-1".
	short currentIndexNodeBlock;

	// The byte buffer used for reading and writing
	// index node blocks.  You can think of this as
	// a one-block cache.
	byte *indexBlockBytes;

public:
	short allocateIndexNode(); // throws IOException
	void freeIndexNode(short node);
	void readIndexNode(IndexNode *indexNode, short indexNodeNumber);
	//   throws IOException
	void writeIndexNode(IndexNode *indexNode , short indexNodeNumber);
	//   throws IOException

private:
	void loadIndexNodeBlock(short indexNodeNumber);  // throws IOException
};

#endif
