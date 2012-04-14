/* FileSystemL: abstraction of a simulated file system */

#include <io_types.h>
#include <BitBlock.h>
#include <FileSystem.h>
#include <Kernel.h>
#include <SuperBlock.h>

/* Construct a FileSystem and open a FileSystem file.
   newFilename is the name of the FileSystem file to open;
   newMode the mode ("r" or "rw") to use when opening the file
*/
FileSystem::FileSystem(String newFilename, String newMode) {
	file = null;
	filename = null;
	mode = null;
	blockSize = 0;
	blockCount = 0;
	freeListBlockOffset = 0;
	inodeBlockOffset = 0;
	dataBlockOffset = 0;
	rootIndexNode = null;

	currentFreeListBitNumber = 0;
	currentFreeListBlock = -1;
	freeListBitBlock = null;

	currentIndexNodeNumber = 0;
	currentIndexNodeBlock = -1;
	indexBlockBytes = null;

	filename = newFilename;
	mode = newMode;
	open();
}

// Get the blockSize for this FileSystem (in bytes)
short FileSystem::getBlockSize()
{
  return blockSize;
}

int FileSystem::getFreeListBlockOffset()
{
  return freeListBlockOffset;
}

int FileSystem::getInodeBlockOffset()
{
  return inodeBlockOffset;
}

int FileSystem::getDataBlockOffset()
{
  return dataBlockOffset;
}

//  Get the rootIndexNode for this FileSystem.
IndexNode *FileSystem::getRootIndexNode() {
	return rootIndexNode;
}

// Open a backing file for this FileSystem and read the superblock.
void FileSystem::open() {
	file = new RandomAccessFile(filename, mode);

	// read the block size and other information from the superblock
	SuperBlock *superBlock = new SuperBlock();
	superBlock->read(file);
	blockSize = SuperBlock::getBlockSize();
	blockCount = superBlock->getBlocks();
	// ??? inodeCount
	freeListBlockOffset = superBlock->getFreeListBlockOffset();
	inodeBlockOffset = superBlock->getInodeBlockOffset();
	dataBlockOffset = superBlock->getDataBlockOffset();

	// initialize free list block buffer
	freeListBitBlock = new BitBlock(blockSize);

	// initialize index block buffer
	indexBlockBytes = new byte[blockSize];

	// read the root index node
	rootIndexNode = new IndexNode();
	readIndexNode(rootIndexNode, ROOT_INDEX_NODE_NUMBER);
}

// Close the backing file for this FileSystem, if any.
void FileSystem::close() {
	if(file != null)
		file->Close();
}

/* Read bytes into a buffer from the specified absolute block number
   of the file system;
   bytes is the byte buffer into which the block should be read;
   blockNumber the absolute block number which should be read
*/
void FileSystem::read(byte *bytes, int blockNumber) {
	file->Seek(blockNumber * blockSize);
	file->readFully(bytes);
}

/* Write bytes from a buffer to the specified absolute block number
   of the file system;
   bytes is the byte buffer from which the block should be written;
   blockNumber the absolute block number which should be written.
*/
void FileSystem::write(byte *bytes, int blockNumber) {
	file->Seek(blockNumber * blockSize);
	file->Write(bytes);
}

/*  Mark a data block as being free in the free list;
    dataBlockNumber is the data block which is to be marked free.
*/
void FileSystem::freeBlock(int dataBlockNumber) {
	loadFreeListBlock(dataBlockNumber);

	freeListBitBlock->resetBit(dataBlockNumber % (blockSize * 8));

	file->Seek((freeListBlockOffset + currentFreeListBlock) *
		 blockSize);
	freeListBitBlock->write(file);
}

/* Allocate a data block from the list of free blocks.
   return the data block number which was allocated; -1 if no blocks
   are available
*/
int FileSystem::allocateBlock() {
	// from our current position in the free list block,
	// scan until we find an open position.  If we get back to
	// where we started, there are no free blocks and we return
	// -1.
	int save = currentFreeListBitNumber;
	while (true) {
		loadFreeListBlock(currentFreeListBitNumber);
		boolean allocated = freeListBitBlock->isBitSet
			(currentFreeListBitNumber % (blockSize * 8));

		int previousFreeListBitNumber = currentFreeListBitNumber;
		currentFreeListBitNumber ++;

		// if curr bit number >= data block count, set to 0
		if(currentFreeListBitNumber >= (blockCount - dataBlockOffset))
		currentFreeListBitNumber = 0;
		if(! allocated) {
			freeListBitBlock->setBit(previousFreeListBitNumber %
					   (blockSize * 8));
			file->Seek((freeListBlockOffset + currentFreeListBlock) *
				 blockSize);

			freeListBitBlock->write(file);
			return previousFreeListBitNumber;
		}

		if(save == currentFreeListBitNumber) {
			Kernel::setErrno(Kernel::ENOSPC);
			return -1;
		}
	}
}

/* Allocate a specific data block from the list of free blocks.
*/
void FileSystem::allocateBlock(int dataBlockNumber) {
	loadFreeListBlock(dataBlockNumber);
	freeListBitBlock->setBit(dataBlockNumber % (blockSize * 8));
	file->Seek((freeListBlockOffset + currentFreeListBlock) * blockSize);
	freeListBitBlock->write(file);
}

/* Loads the block containing the specified data block bit into
   the free list block buffer.  This is a convenience method;
   dataBlockNumber is the data block number.
*/
void FileSystem::loadFreeListBlock(int dataBlockNumber) {
	int neededFreeListBlock = dataBlockNumber / (blockSize * 8);
	if(currentFreeListBlock != neededFreeListBlock) {
		file->Seek((freeListBlockOffset + neededFreeListBlock) *
		 blockSize);
		freeListBitBlock->read(file);
		currentFreeListBlock = neededFreeListBlock;
	}
}


/* Allocate an index node for the file system.
   return the inode number for the next available index node;
   -1 if there are no index nodes available.
*/
short FileSystem::allocateIndexNode()
// throws IOException
{
	// from our current position in the index node block list,
	// scan until we find an open position.  If we get back to
	// where we started, there are no free inodes and we return
	// -1.
	short save = currentIndexNodeNumber;
	IndexNode *temp = new IndexNode();

	while (true) {
		readIndexNode(temp, currentIndexNodeNumber);
		short previousIndexNodeNumber = currentIndexNodeNumber;
		++currentIndexNodeNumber;
		// if curr inode >= avail inode space, set to 0
		if(currentIndexNodeNumber >=
			((dataBlockOffset - inodeBlockOffset) *
			(blockSize / IndexNode::INDEX_NODE_SIZE)))
			currentIndexNodeNumber = 0; //Wrap around
			if(temp->getNlink() == 0) {
				// ??? should we update nlinks here?
				return previousIndexNodeNumber;
			}
			if(save == currentIndexNodeNumber) {
				// ??? it seems like we should give a different error here
				Kernel::setErrno(Kernel::ENOSPC);
				return -1;
			}
	}
}

void FileSystem::freeIndexNode(short node) {
	fprintf(stderr, "Freeing resources for node %d\n", node);
	IndexNode *temp = new IndexNode();
	readIndexNode(temp, node);

	int numBlocks = (temp->getSize() / blockSize);
	numBlocks += (temp->getSize() % blockSize) > 0 ? 1 : 0;
	fprintf(stderr, "File Size: %d\n", temp->getSize());

	for (int i = 0; i < numBlocks; ++i) {
		fprintf(stderr, "Freeing physical block %d\n", temp->getBlockAddress(i));
		freeBlock(temp->getBlockAddress(i));

		temp->setBlockAddress(i, FileSystem::NOT_A_BLOCK);
	}

	//Not necessary but a potential optimization
	if ( currentIndexNodeNumber == (node - 1) )
		--currentIndexNodeNumber;

	temp->setNlink(0);
	temp->setSize(0);
	writeIndexNode(temp, node);
	delete temp;

	return;
}

int FileSystem::freeInodeBlocks(short node, short howmany) {
	IndexNode *temp = new IndexNode();
	readIndexNode(temp, node);

	int numBlocks = ((temp->getSize() - 1) / blockSize);
	if ( numBlocks < howmany ) {
		Kernel::setErrno(Kernel::EINVAL);
		return -1;
	}

	for ( int i = 0; i < howmany; ++i) {
		freeBlock(temp->getBlockAddress(numBlocks-i-1));
		temp->setBlockAddress(numBlocks-i-1, FileSystem::NOT_A_BLOCK);
	}

	writeIndexNode(temp, node);
	delete temp;

	return 0;
}

  /* Reads an index node at the index node location specified;
     indexNode the index node;
     indexNodeNumber the location.
   */
void FileSystem::readIndexNode(IndexNode *indexNode, short indexNodeNumber) {
	loadIndexNodeBlock(indexNodeNumber);

	/* When loaded we may have more than one inode in a
	 * block so index appropriately
	 */
	indexNode->read(indexBlockBytes,
		  (indexNodeNumber * IndexNode::INDEX_NODE_SIZE) %
		  blockSize);
}

/* Writes an index node at the index node location specified;
   indexNode the index node;
   indexNodeNumber the location.
*/
void FileSystem::writeIndexNode(IndexNode *indexNode, short indexNodeNumber) {
  loadIndexNodeBlock(indexNodeNumber);

  indexNode->write(indexBlockBytes,
		   (indexNodeNumber * IndexNode::INDEX_NODE_SIZE) %
		   blockSize);

  write(indexBlockBytes, inodeBlockOffset + currentIndexNodeBlock);
}

/* Loads the block containing the specified index node into
   the index node block buffer.  This is a convenience method;
   indexNodeNumber is the index node number.
*/
void FileSystem::loadIndexNodeBlock(short indexNodeNumber) {
	short neededIndexNodeBlock = (short)(indexNodeNumber /
					   (blockSize / IndexNode::INDEX_NODE_SIZE));

	if(currentIndexNodeBlock != neededIndexNodeBlock) {
		read(indexBlockBytes, inodeBlockOffset + neededIndexNodeBlock);
		currentIndexNodeBlock = neededIndexNodeBlock;
	}
}

int FileSystem::getTakenDBlocks() {
	int countSet = 0;
	BitBlock *freeList;
	BitBlock *freeListPrev = NULL;
	int numDataBlocks = getBlockCount() - getDataBlockOffset();
	for (int i  = 0; i < numDataBlocks; ++i) {
		freeList = getFreeList(i);
		if (freeList != freeListPrev){
			countSet+= freeList->countSet();
		}
		freeListPrev = freeList;
	}
	return countSet;
}

int FileSystem::getTakenInodes() {
	IndexNode *temp = new IndexNode();

	int end = ((dataBlockOffset - inodeBlockOffset) *
			(blockSize / IndexNode::INDEX_NODE_SIZE));

	int count = 0;

	for ( int i = 0; i < end; ++i ) {
		readIndexNode(temp, i);

		if ( temp->getNlink() != 0 )
			++count;
	}

	delete temp;
	return count;
}

int FileSystem::getBlockCount() {
	return blockCount;
}

int FileSystem::getInodeCount() {
	return ((dataBlockOffset - inodeBlockOffset) *
			(blockSize / IndexNode::INDEX_NODE_SIZE));
}

BitBlock * FileSystem::getFreeList(int bit){
	loadFreeListBlock(bit);
	return freeListBitBlock;
}
