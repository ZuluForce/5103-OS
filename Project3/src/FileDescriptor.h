/* FileDescriptor: abstraction of file descriptors in the simulated file
   system.  */

#ifndef FD_H
#define FD_H

#include <IndexNode.h>
#include <FileSystem.h>

class FileDescriptor {
private:
	FileSystem *fileSystem;
	IndexNode *indexNode;
	short deviceNumber;
	short indexNodeNumber ;
	int flags;
	int offset;
	byte *bytes;

public:
	FileDescriptor();
	FileDescriptor(short newDeviceNumber, short newIndexNodeNumber,
		 int newFlags);
	FileDescriptor( FileSystem *newFileSystem, IndexNode *newIndexNode,
		  int newFlags);
	void setDeviceNumber(short newDeviceNumber );
	short getDeviceNumber();
	IndexNode *getIndexNode();
	void setIndexNodeNumber( short newIndexNodeNumber );
	short getIndexNodeNumber();
	int getFlags();
	byte* getBytes();
	short getMode();
	int getSize();
	void setSize(int newSize );// throws IOException
	short getBlockSize();
	int getOffset();
	void setOffset(int newOffset);
	int readBlock(short relativeBlockNumber);
	int writeBlock( short relativeBlockNumber ); //  throws Exception
};

#endif
