/* Block: Abstraction of a block of bytes */

#ifndef BLOCK_H
#define BLOCK_H

#include <io_types.h>
#include <RandomAccessFile.h>

class Block 
{
 public:
  short blockSize; // the block size in bytes for this block
  byte *bytes; // the array of bytes for this block
  
 public:
  Block(); // construct a block
  Block(short blockSize); // Construct a block with a given block size.
  void setBlockSize(short newBlockSize);
  short getBlockSize();

  // Read a block from a file at the current position.
  // I/O exceptions can occur
  void read(RandomAccessFile *file); // throws IOException , EOFException

  // Write a block to a file at the current position.
  // I/O exceptions can occur
  void write(RandomAccessFile *file);// throws IOException
};

#endif
