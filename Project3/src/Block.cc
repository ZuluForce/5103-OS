/* Block: Abstraction of a block of bytes */

#include <Block.h>

Block::Block()
{
  blockSize = 0;
  bytes = null;
}

Block::Block(short blockSize)
{
  bytes = null;
  setBlockSize(blockSize);
}

void Block::setBlockSize(short newBlockSize)
{
  blockSize = newBlockSize;
  bytes = new byte[blockSize];
} 

short Block::getBlockSize()
{
  return blockSize;
}

void Block::read(RandomAccessFile *file) 
//throws IOException , EOFException
{
  file->readFully(bytes);
}

void Block::write(RandomAccessFile *file) 
//throws IOException
{
  file->Write(bytes);
}

