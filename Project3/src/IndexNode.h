/* IndexNode: abstraction of an index node for a simulated file system */

#ifndef INDEX_NODE_H
#define INDEX_NODE_H

#include <io_types.h>

class IndexNode
{
 public:
  static const int INDEX_NODE_SIZE = 64 ; // index node size in bytes
  static const int MAX_DIRECT_BLOCKS = 10 ; // max direct blocks in an index node

  /* Maximum number of blocks in a file.  If indirect,
     doubleIndirect, or tripleIndirect blocks are implemented,
     this number will need to be increased.   */
   static const int MAX_FILE_BLOCKS = MAX_DIRECT_BLOCKS ;

 private:
   // Mode for this index node.  This includes file type and file protection
   // information.
   short mode;

   // Not yet implemented. Number of links to this file.
   short nlink;

   // Not yet implemented. Owner's user id.
   short uid;

   // Not yet implemented. Owner's group id.
   short gid;

   int size; // Number of bytes in this file.

  /* Array of direct blocks containing the block addresses for the
     first MAX_DIRECT_BLOCKS blocks of the file.  Note that each
     element in the array is stored as a 3-byte number on disk.
  */
  int directBlocks[MAX_DIRECT_BLOCKS];

  // Not yet implemented.
  int indirectBlock;

  // Not yet implemented.
  int doubleIndirectBlock;

  // Not yet implemented.
  int tripleIndirectBlock;

  /* Not yet implemented.  The date and time at which this file was last accessed.
     This is traditionally implemented as the number of seconds
     past 1970/01/01 00:00:00
  */
  int atime;

  /* Not yet implemented. The date and time at which this file was last modified.
     This is traditionally implemented as the number of seconds
     past 1970/01/01 00:00:00
  */
  int mtime;

  /* Not yet implemented. The date and time at which this file was created.
     This is traditionally implemented as the number of seconds
     past 1970/01/01 00:00:00
  */

  int ctime;

public:
	IndexNode();
	void setMode(short newMode);
	short getMode();
	void setNlink(short newNlink);
	short getNlink();
	bool incNlink();
	bool decNlink();
	void setUid(short newUid);
	short getUid();
	short getGid();
	void setGid(short newGid);
	void setSize(int newSize);
	int getSize();
	int getBlockAddress(int block); // throws Exception
	void setBlockAddress(int block, int address); // throws Exception
	void setAtime(int newAtime);
	int getAtime();
	void setMtime(int newMtime);
	int getMtime();
	void setCtime(int newCtime);
	int getCtime();
	void write(byte *buffer, int offset);
	void read(byte *buffer, int offset);
	String toString();
	void copy(IndexNode *indexNode);
	//*** These methods should be implemented for indirect blocks ***
	int initIndirectBlock(byte *indirBlock);
	int deserializeBlockNumber(byte *indirBlock, int offset);
	void serializeBlockNumber(byte *indirBlock, int offset, int value);
};

#endif
