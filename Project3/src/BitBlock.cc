/* Block: Abstraction of a block of bits */

#include <BitBlock.h>

BitBlock::BitBlock(short blockSize): Block(blockSize) { }

// Set a specified (whichBit) bit to 1 (true)
void BitBlock::setBit(int whichBit) {
  bytes[whichBit/8] |= (byte)(1 << (whichBit%8));
}

// Set a specified (whichBit) bit to value
void BitBlock::setBit(int whichBit, boolean value) {
  if (value)
    setBit(whichBit);
  else
    resetBit(whichBit);
}

// Checks to see if whichBit is set (1)
boolean BitBlock::isBitSet(int whichBit) {
  return (bytes[whichBit/8] & (byte)(1 << (whichBit%8))) != 0;
}

// Set a specified (whichBit) bit to false (0)
void BitBlock::resetBit(int whichBit) {
  bytes[whichBit/8] &= ~ (byte)(1 << (whichBit%8));
}

int BitBlock::countSet() {
	int cnt = 0;
	for (int i = 0; i < blockSize; ++i) {
		if ( isBitSet(i) )
			++cnt;
	}

	return cnt;
}
