#include <Kernel.h>
#include <iostream>
using namespace std;
static const String PROGRAM_NAME = "fsck";

// The size of the buffer to be used when reading files.
static const int BUF_SIZE = 4096;

// The file mode to use when creating the output file.
// ??? perhaps this should be the same mode as the input file
static const short OUTPUT_MODE = 0700;

int numINodes;
int *linkCounts;
int *dataBlocks;
int numDataBlocks;

int checkDirContents(int fd, String path){

    int status;
    int subFd;
    DirectoryEntry *directoryEntry = new DirectoryEntry();
    Stat *stat = new Stat();
    String name;

    while (1){
        // read an entry; quit loop if error or nothing read
        status = Kernel::readdir(fd, directoryEntry);
        if(status <= 0)
            break;
        name = directoryEntry->getName();
        linkCounts[directoryEntry->getIno()]++;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
        	continue;
        }
        StringBuffer *newPathBuilder = new StringBuffer(path);
        newPathBuilder->append(name);
        String newPath = newPathBuilder->toString();
        status = Kernel::stat(newPath, stat, true);

        if (status < 0){
            fprintf(stderr, "stat failed!\n");
            break;
        }
        if ((stat->getMode() & Kernel::S_IFMT) == Kernel::S_IFDIR){
            subFd = Kernel::open(newPath, Kernel::O_RDONLY);
            if (subFd < 0){
                Kernel::perror(PROGRAM_NAME);
                fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
                   ": unable to open ", newPath, " for reading");
                Kernel::Exit(1);
            }
            newPathBuilder->append("/");
            checkDirContents(subFd, newPathBuilder->toString());
        }
        //delete newPath;
        //delete newPathBuilder;

    }
    // check to see if our last read failed
    if(status < 0){
        Kernel::perror("checkDirContents");
        fprintf(stderr, "checkDirContents: unable to read directory entry\n");
       exit(1);
    }

    // close the directory
    Kernel::close(fd);
    return 0;
}

/* fsck command */
int main(int argc, char **argv)
// throws Exception
{
  // initialize the file system simulator kernel
    Kernel::initialize();

    int silentMode = false;

    if ( argc >= 2 && !strcmp(argv[1], "-silent") ) {
    	silentMode = true;
    }

    int numNodeErrors = 0;
    int numBlockErrors = 0;

    FileSystem *fs = Kernel::openFileSystems[Kernel::ROOT_FILE_SYSTEM];
    short blockSize = fs->getBlockSize();
    numINodes = fs->getInodeCount();
    linkCounts = new int[numINodes];
    int *newLinkCounts = new int[numINodes];
    for (int i = 0; i < numINodes; i++){
    	newLinkCounts[i] = -1;
    }
    linkCounts[0] = 1;

    int fd = Kernel::open("/", Kernel::O_RDONLY);
    if(fd < 0) {
			Kernel::perror(PROGRAM_NAME);
			fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
			   ": unable to open ", "/", " for reading");
			Kernel::Exit(1);
    }
    printf("Step 1: Scanning the filesystem...\n");
    checkDirContents(fd, "/");
    printf("Scanning complete.\n");
    IndexNode *temp = new IndexNode();
    for (int i = 0; i < numINodes; i++){
        fs->readIndexNode(temp, i);
        if (temp->getNlink() > linkCounts[i]){
        	numNodeErrors++;
        	newLinkCounts[i] = linkCounts[i];
        	if (linkCounts[i] == 0){
				printf("There are no more links to IndexNode %d on the filesystem yet it is still allocated.\n", i);
				printf("Fixing this error will deallocate this IndexNode and all of its blocks.\n");
        	} else{
				printf("IndexNode %d reports more links (%d) to it than are found on the filesystem (%d).\n", i, temp->getNlink(), linkCounts[i]);
				printf("Fixing this error will set the number of links to %d.\n", linkCounts[i]);
        	}
        } else if (linkCounts[i] > temp->getNlink()){
        	numNodeErrors++;
        	newLinkCounts[i] = linkCounts[i];
			printf("IndexNode %d reports fewer links (%d) to it than are found on the filesystem (%d).\n", i, temp->getNlink(), linkCounts[i]);
			printf("Fixing this error will set the number of links to %d.\n", linkCounts[i]);
        }
    }

    printf("fsck found %d IndexNode link errors on the filesystem.\n", numNodeErrors);
	if (numNodeErrors > 0){
		char answer;
		if (!silentMode){
			do{
				cout << "Would you like to have fsck repair the errors? [y/n]" << endl;
				cin >> answer;
			}
			while( !cin.fail() && answer != 'y' && answer != 'n');
		} else{
			answer = 'y';
		}
		if (answer == 'y'){
			printf("Fixing IndexNode counts...\n");
			for (int i = 0; i < numINodes; i++){
				if (newLinkCounts[i] >= 0){
					if (newLinkCounts[i] == 0){
						fs->freeIndexNode(i);
					} else{
						fs->readIndexNode(temp, i);
						temp->setNlink(newLinkCounts[i]);
						fs->writeIndexNode(temp, i);
					}
				}
			}
			printf("Fixing complete.\n");
		}
	}


	printf("Step 2: Scanning the filesystem...\n");
	printf("Scanning complete.\n");

    numDataBlocks = fs->getBlockCount() - fs->getDataBlockOffset();
    dataBlocks = new int[numDataBlocks];
    bool *blocksToFree = new bool[numDataBlocks];
    memset((void*)blocksToFree, '\0', sizeof(bool) * numDataBlocks);
    bool *blocksToAllocate = new bool[numDataBlocks];
    memset((void*)blocksToAllocate, '\0', sizeof(bool) * numDataBlocks);
    for (int i = 0; i < numDataBlocks; i++){
    	dataBlocks[i] = -1;
    }


	for ( int i = 0; i < numINodes; i++ ) {
		fs->readIndexNode(temp, i);
		if ( temp->getNlink() != 0 ){
			for (int j = 0; j < IndexNode::MAX_DIRECT_BLOCKS; j++)
				if (temp->getBlockAddress(j) != FileSystem::NOT_A_BLOCK){
					dataBlocks[temp->getBlockAddress(j)] = i;
					//printf("Block %d for inode %d is allocated at address %d.\n", j, i, temp->getBlockAddress(j));
				}
		}

	}

	BitBlock *freeList;
	/*for (int i = 0; i < numDataBlocks; i++){
		freeList = fs->getFreeList(i);
		if (freeList->isBitSet(i) == true){
					printf("Address %d is allocated according to free list.\n", i);
				} else{
					printf("Address %d is free according to free list.\n", i);
				}
	}*/

	for (int i = 0; i < numDataBlocks; i++){
		freeList = fs->getFreeList(i);
		if ((dataBlocks[i] != -1) && ! freeList->isBitSet(i % (blockSize * 8))){
			numBlockErrors++;
			blocksToAllocate[i] = true;
			printf("Physical block %d is in use by IndexNode %d but it is not allocated on the filesystem.\n", i, dataBlocks[i]);
			printf("Fixing this error will allocate it in the filesystem again.\n");
		} else if ((dataBlocks[i] == -1) && freeList->isBitSet(i % (blockSize * 8))){
			numBlockErrors++;
			blocksToFree[i] = true;
			printf("Physical block %d is allocated on the filesystem but is not used by any IndexNode.\n", i);
			printf("Fixing this error will deallocate it in the filesystem.\n");
		}
	}

	printf("fsck found %d block errors on the filesystem.\n", numBlockErrors);

	if (numBlockErrors > 0){
		char answer;
		if (!silentMode){
			do{
				cout << "Would you like to have fsck repair the errors? [y/n]" << endl;
				cin >> answer;
			}
			while( !cin.fail() && answer != 'y' && answer != 'n');
		} else{
			answer = 'y';
		}
		if (answer == 'y'){
			printf("Fixing data block allocation...\n");
			for (int i = 0; i < numDataBlocks; i++){
				if (blocksToFree[i]){
					printf("Deallocating physical block %d on the filesystem.\n", i);
					fs->freeBlock(i);
				}
				if (blocksToAllocate[i]){
					printf("Allocating physical block %d on the filesystem.\n", i);
					fs->allocateBlock(i);
				}
			}
			printf("Fixing complete.\n");



		}
	}




    Kernel::Exit(0);

}

