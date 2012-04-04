/* SuperBlock: Abstraction for file system super block */

#include <SuperBlock.h>

short SuperBlock::blockSize;

SuperBlock::SuperBlock()
{
}

void SuperBlock::setBlockSize(short newBlockSize)
{
  blockSize = newBlockSize;
}

short SuperBlock::getBlockSize()
{
  return (SuperBlock::blockSize);
}

void SuperBlock::setBlocks(int newBlocks)
{
  blocks = newBlocks;
}

int SuperBlock::getBlocks()
{
  return blocks;
}

void SuperBlock::setFreeListBlockOffset(int newFreeListBlockOffset)
{
  freeListBlockOffset = newFreeListBlockOffset;
}

int SuperBlock::getFreeListBlockOffset()
{
  return freeListBlockOffset;
}

void SuperBlock::setInodeBlockOffset(int newInodeBlockOffset)
{
  inodeBlockOffset = newInodeBlockOffset;
}

int SuperBlock::getInodeBlockOffset()
{
  return inodeBlockOffset;
}

void SuperBlock::setDataBlockOffset(int newDataBlockOffset)
{
  dataBlockOffset = newDataBlockOffset;
}

int SuperBlock::getDataBlockOffset()
{
  return dataBlockOffset;
}

void SuperBlock::write(RandomAccessFile *file) // throws IOException
{
  file->writeShort(blockSize);
  file->writeInt(blocks);
  file->writeInt(freeListBlockOffset);
  file->writeInt(inodeBlockOffset);
  file->writeInt(dataBlockOffset);
  for(int i = 0; i < blockSize - 18; i ++)
    file->Write((byte) 0);
}

void SuperBlock::read(RandomAccessFile *file) // throws IOException
{
  blockSize = file->readShort();
  blocks = file->readInt();
  freeListBlockOffset = file->readInt();
  inodeBlockOffset = file->readInt();
  dataBlockOffset = file->readInt();
  file->skipBytes(blockSize - 18);
}
