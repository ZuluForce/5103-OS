/* FileDescriptor: abstraction of file descriptors in the simulated file
   system.  */

#include <FileDescriptor.h>
#include <Kernel.h>

FileDescriptor::FileDescriptor()
// throws IOException
{}


FileDescriptor::FileDescriptor(short newDeviceNumber,
			       short newIndexNodeNumber, int newFlags)
// throws IOException
  {
    fileSystem = null;
    indexNode = null;
    deviceNumber = -1;
    indexNodeNumber = -1;
    flags = 0;
    offset = 0;
    bytes = null ;
    deviceNumber = newDeviceNumber;
    indexNodeNumber = newIndexNodeNumber;
    flags = newFlags;
    fileSystem = Kernel::openFileSystems[deviceNumber];
    indexNode = new IndexNode();
    fileSystem->readIndexNode(indexNode, indexNodeNumber);
    bytes = new byte[getBlockSize()] ;
  }

FileDescriptor::FileDescriptor(FileSystem *newFileSystem,
			       IndexNode *newIndexNode, int newFlags)
{
  fileSystem = null;
  indexNode = null;
  deviceNumber = -1;
  short indexNodeNumber = -1;
  flags = 0;
  offset = 0;
  bytes = null;
  fileSystem = newFileSystem;
  indexNode = newIndexNode;
  flags = newFlags;
  bytes = new byte[fileSystem->getBlockSize()];
}

void FileDescriptor::setDeviceNumber(short newDeviceNumber) {
  deviceNumber = newDeviceNumber;
}

short FileDescriptor::getDeviceNumber() {
  return deviceNumber;
}

IndexNode* FileDescriptor::getIndexNode() {
  return indexNode;
}

void FileDescriptor::setIndexNodeNumber(short newIndexNodeNumber) {
  indexNodeNumber = newIndexNodeNumber;
}

short FileDescriptor::getIndexNodeNumber() {
  return indexNodeNumber;
}

int FileDescriptor::getFlags() {
  return flags;
}

byte* FileDescriptor::getBytes() {
  return bytes;
}

short FileDescriptor::getMode() {
  return indexNode->getMode();
}

int FileDescriptor::getSize() {
  return indexNode->getSize();
}

void FileDescriptor::setSize(int newSize) // throws IOException
{
  indexNode->setSize(newSize);

  // write the inode
  fileSystem->writeIndexNode(indexNode , indexNodeNumber);
}

short FileDescriptor::getBlockSize() {
  return fileSystem->getBlockSize();
}

int FileDescriptor::getOffset() {
  return offset;
}

void FileDescriptor::setOffset(int newOffset) {
  offset = newOffset;
}

int FileDescriptor::readBlock(short relativeBlockNumber) {
  if(relativeBlockNumber >= IndexNode::MAX_FILE_BLOCKS) {
		Kernel::setErrno(Kernel::EFBIG);
		return -1;
    }
  // ask the IndexNode for the actual block number
  // given the relative block number
  int blockOffset =
    indexNode->getBlockAddress(relativeBlockNumber);

  if(blockOffset == FileSystem::NOT_A_BLOCK)
    {
      // clear the bytes if it's a block that was never written
      int blockSize = fileSystem->getBlockSize();
      for(int i = 0; i < blockSize; i++)
        bytes[i] = (byte)0;
    }
  else
    {
      // read the actual block into bytes
      fileSystem->read(bytes,
		       fileSystem->getDataBlockOffset() + blockOffset);
    }
  return 0;
}

int FileDescriptor::writeBlock(short relativeBlockNumber)
// throws Exception
{
  if(relativeBlockNumber >= IndexNode::MAX_FILE_BLOCKS)
    {
      Kernel::setErrno(Kernel::EFBIG);
      return -1;
    }
  // ask the IndexNode for the actual block number
  // given the relative block number
  int blockOffset =
    indexNode->getBlockAddress(relativeBlockNumber);

  if(blockOffset == FileSystem::NOT_A_BLOCK)
    {
      // allocate a block; quit if we can't
      blockOffset = fileSystem->allocateBlock();
      if(blockOffset < 0)
        return -1;

      // update the inode
      indexNode->setBlockAddress(relativeBlockNumber, blockOffset);
      // write the inode
      fileSystem->writeIndexNode(indexNode, indexNodeNumber);
    }

  // write the actual block from bytes
  fileSystem->write(bytes,
		    fileSystem->getDataBlockOffset() + blockOffset);
  return 0;
}

