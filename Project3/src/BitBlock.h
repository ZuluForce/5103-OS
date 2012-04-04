/* Block: Abstraction of a block of bits */

#ifndef BIT_BLOCK_H
#define BIT_BLOCK_H

#include <Block.h>
class BitBlock : public Block
{
 public:
  void setBit(int);
  void setBit(int, boolean);
  boolean isBitSet(int);
  void resetBit(int);
  BitBlock(short blockSize);
  BitBlock( );
};

#endif
