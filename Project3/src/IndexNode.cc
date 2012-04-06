/* IndexNode: abstraction of an index node for a simulated file system */

#include <IndexNode.h>
#include <FileSystem.h>

IndexNode::IndexNode()
{
  mode = 0;
  nlink  = 0;
  uid = 0;
  gid = 0;
  size = 0;
  for (int i=0; i<MAX_DIRECT_BLOCKS; i++)
    directBlocks [i] = FileSystem::NOT_A_BLOCK;

  indirectBlock = FileSystem::NOT_A_BLOCK;
  doubleIndirectBlock = FileSystem::NOT_A_BLOCK;
  tripleIndirectBlock = FileSystem::NOT_A_BLOCK;
  atime = 0;
  mtime = 0;
  ctime = 0;
}


// Sets the mode for this IndexNode.
// This is the file type and file protection information.
void IndexNode::setMode(short newMode)
{
  mode = newMode;
}


// Gets the mode for this IndexNode.
// This is the file type and file protection information.
short IndexNode::getMode()
{
  return mode;
}

// Set the number of links for this IndedNode.
//  newNlink is the number of links
void IndexNode::setNlink(short newNlink)
{
  nlink = newNlink;
}


// Get the number of links for this IndexNode.
short IndexNode::getNlink()
{
  return nlink;
}

bool IndexNode::incNlink() {
	//Could chedk if the short is going to overflow
	++nlink;

	return true;
}

bool IndexNode::decNlink() {
	if ( nlink == 0 ) {
		return false;
	}
	--nlink;
	return true;
}

void IndexNode::setUid(short newUid)
{
  uid = newUid;
}

short IndexNode::getUid()
{
  return uid;
}

short IndexNode::getGid()
{
  return gid;
}

void IndexNode::setGid(short newGid)
{
  gid = newGid;
}


// Sets the size for this IndexNode.
// This is the number of bytes in the file.
void IndexNode::setSize(int newSize)
{
  size = newSize;
}

// Gets the size for this IndexNode.
// This is the number of bytes in the file.
int IndexNode::getSize()
{
  return size;
}

/* Create and initialize an indirect inode block. This is done by setting
 * all the addresses within the block to NOT_A_BLOCK
 * The number of the newly created indirect block is returned
 * */
int IndexNode::initIndirectBlock(byte *indirBlock)
{
	//Allocate the indirect block

	//Initialize it by setting all the indirect block addresses to Not_a_block

	// save this indirect block to disk

	//return the allocated block number
	return 0;
}

/* Read the block number at the 3 bytes from offset and return it
 * Inputs: indirBlock: Byte array which holds the block data from which the block number is read
 * offset: offset within the block from which to read the block number
 * returns: the block number read
 * */
int IndexNode::deserializeBlockNumber(byte *indirBlock, int offset)
{
	//look at the IndexNode::Read() function on ideas on how to implement this
}

/* Write  the block number (given by 'value') at the 3 bytes from offset and return it
 * Inputs: indirBlock: Byte array which holds the block data into which the block number is written
 * offset: offset within the block whre the block number is written
 * value: block number to write to the byte array
 * */
void IndexNode::serializeBlockNumber(byte *indirBlock, int offset, int value)
{
	//look at the IndexNode::Write() function on ideas on how to implement this
}


/* Gets the address corresponding to the specified
   sequential block of the file;
   block is the sequential block number
   returns the address of the block, a number between zero and one
   less than the number of blocks in the file system
*/
int IndexNode::getBlockAddress(int block)
// throws Exception
{
  if(block >= 0 && block < MAX_DIRECT_BLOCKS)
    return(directBlocks[block]);
  else
    //      throw new Exception("invalid block address " + block);
    throw "invalid block address";
}

/* Sets the address corresponding to the specified sequential
   block of the file;
   block is the sequential block number;
   address the address of the block, a number between zero and one
   less than the number of blocks in the file system
*/
void IndexNode::setBlockAddress(int block, int address)
// throws Exception
{
  if(block >= 0 && block < MAX_DIRECT_BLOCKS)
    directBlocks[block] = address;
  else
    throw "invalid block address";
  //new Exception("invalid block address " + block);
}

void IndexNode::setAtime(int newAtime)
{
  atime = newAtime;
}

int IndexNode::getAtime()
{
  return atime;
}

void IndexNode::setMtime(int newMtime)
{
  mtime = newMtime;
}

int IndexNode::getMtime()
{
  return mtime;
}

void IndexNode::setCtime(int newCtime)
{
  ctime = newCtime;
}

int IndexNode::getCtime()
{
  return ctime;
}

/* Writes the contents of an index node to a byte array.
   This is used to copy the bytes which correspond to the
   disk image of the index node onto a block buffer so that
   they may be written to the file system;
   buffer is the buffer to which bytes should be written;
   offset the offset from the beginning of the buffer
   at which bytes should be written
*/
void IndexNode::write(byte *buffer, int offset)
{
  // write the mode info
  buffer[offset] = (byte)(mode >> 8);
  buffer[offset+1] = (byte)mode;

  // write nlink
  buffer[offset+2] = (byte)(nlink >> 8);
  buffer[offset+3] = (byte)nlink;

  // write uid
  buffer[offset+4] = (byte)(uid >> 8);
  buffer[offset+5] = (byte)uid;

  // write gid
  buffer[offset+6] = (byte)(gid >> 8);
  buffer[offset+7] = (byte)gid;

  // write the size info
  buffer[offset+8]   = (byte)(size >> 24);
  buffer[offset+8+1] = (byte)(size >> 16);
  buffer[offset+8+2] = (byte)(size >> 8);
  buffer[offset+8+3] = (byte)(size);

  // write the directBlocks info 3 bytes at a time
  for(int i = 0; i < MAX_DIRECT_BLOCKS; i ++)
    {
      buffer[offset+12+3*i]   = (byte)(directBlocks[i] >> 16);
      buffer[offset+12+3*i+1] = (byte)(directBlocks[i] >> 8);
      buffer[offset+12+3*i+2] = (byte)(directBlocks[i]);
    }

  // leave room for indirectBlock, doubleIndirectBlock, tripleIndirectBlock

  // leave room for atime, mtime, ctime
  }

/* Reads the contents of an index node from a byte array.
   This is used to copy the bytes which correspond to the
   disk image of the index node from a block buffer that
   has been read from the file system;
   buffer the buffer from which bytes should be read;
   offset the offset from the beginning of the buffer
   at which bytes should be read
*/
void IndexNode::read(byte *buffer, int offset)
{
  int b3;
  int b2;
  int b1;
  int b0;

  // read the mode info
  b1 = buffer[offset] & 0xff;
  b0 = buffer[offset+1] & 0xff;
  mode = (short)(b1 << 8 | b0);

  // read the nlink info
  b1 = buffer[offset+2] & 0xff;
  b0 = buffer[offset+3] & 0xff;
  nlink = (short)(b1 << 8 | b0);

  // read the uid info
  b1 = buffer[offset+4] & 0xff;
  b0 = buffer[offset+5] & 0xff;
  uid = (short)(b1 << 8 | b0);

  // read the gid info
  b1 = buffer[offset+6] & 0xff;
  b0 = buffer[offset+7] & 0xff;
  gid = (short)(b1 << 8 | b0);

  // read the size info
  b3 = buffer[offset+8] & 0xff;
  b2 = buffer[offset+8+1] & 0xff;
  b1 = buffer[offset+8+2] & 0xff;
  b0 = buffer[offset+8+3] & 0xff;
  size = b3 << 24 | b2 << 16 | b1 << 8 | b0;

  // read the block address info 3 bytes at a time
  for(int i = 0; i < MAX_DIRECT_BLOCKS; i ++)
    {
      b2 = buffer[offset+12+i*3] & 0xff;
      b1 = buffer[offset+12+i*3+1] & 0xff;
      b0 = buffer[offset+12+i*3+2] & 0xff;
      directBlocks[i] = b2 << 16 | b1 << 8 | b0;
    }

  // leave room for indirectBlock, doubleIndirectBlock, tripleIndirectBlock

  // leave room for atime, mtime, ctime
}


String IndexNode::toString()
{
  StringBuffer *s = new StringBuffer("IndexNode[");
  s->append(mode);
  s->append(',');
  s->append('{');
  for(int i = 0; i < MAX_DIRECT_BLOCKS; i ++)
    {
      if(i > 0)
        s->append(',');
      s->append(directBlocks[i]);
    }
  s->append('}');
  s->append(']');
  return s->toString();
}

void IndexNode::copy(IndexNode *indexNode)
{
  indexNode->mode = mode;
  indexNode->nlink = nlink;
  indexNode->uid = uid;
  indexNode->gid = gid;
  indexNode->size = size;
  for(int i = 0; i < MAX_DIRECT_BLOCKS; i ++)
    indexNode->directBlocks[i] = directBlocks[i];
  indexNode->indirectBlock = indirectBlock;
  indexNode->doubleIndirectBlock = doubleIndirectBlock;
  indexNode->tripleIndirectBlock = tripleIndirectBlock;
  indexNode->atime = atime;
  indexNode->mtime = mtime;
  indexNode->ctime = ctime;
  }


