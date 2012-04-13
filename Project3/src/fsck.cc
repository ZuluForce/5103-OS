#include <Kernel.h>

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
        printf("Looking in: %s\n", path);
        // read an entry; quit loop if error or nothing read
        status = Kernel::readdir(fd, directoryEntry);
        if(status <= 0)
            break;
        name = directoryEntry->getName();
         printf("%s has inode of %d\n", name, directoryEntry->getIno());
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

void printCounts(){
    for (int i = 0; i < numINodes; i++){
        printf("Spot %d has count of %d.\n", i, linkCounts[i]);
    }
}

/* fsck command */
int main(int argc, char **argv)
// throws Exception
{
  // initialize the file system simulator kernel
    Kernel::initialize();

    FileSystem *fs = Kernel::openFileSystems[Kernel::ROOT_FILE_SYSTEM];
    numINodes = fs->getInodeCount();
    printf("%d\n", numINodes);
    linkCounts = new int[numINodes];
    linkCounts[0] = 1;

    int fd = Kernel::open("/", Kernel::O_RDONLY);
    if(fd < 0) {
			Kernel::perror(PROGRAM_NAME);
			fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
			   ": unable to open ", "/", " for reading");
			Kernel::Exit(1);
    }

    checkDirContents(fd, "/");
    IndexNode *temp = new IndexNode();
    for (int i = 0; i < numINodes; i++){
        fs->readIndexNode(temp, i);
        printf("There are %d links to inode %d and the inode internally maintains %d links.\n", linkCounts[i], i, temp->getNlink());
        if (temp->getNlink() != linkCounts[i]){
            //printf("Discrepancy: There are %d links to inode %d but the inode internally maintains %d links.\n", linkCounts[i], i, temp->getNlink());
        }
    }





    numDataBlocks = fs->getBlockCount() - fs->getDataBlockOffset();
    dataBlocks = new int[numDataBlocks];
    memset((void*) dataBlocks, '\0', sizeof(int) * numDataBlocks);


	int end = ((fs->getDataBlockOffset() - fs->getInodeBlockOffset()) *
			(fs->getBlockSize() / IndexNode::INDEX_NODE_SIZE));


	for ( int i = 0; i < end; ++i ) {
		fs->readIndexNode(temp, i);
		if ( temp->getNlink() != 0 ){
			for (int j = 0; j < IndexNode::MAX_DIRECT_BLOCKS; j++)
				if (temp->getBlockAddress(j) != FileSystem::NOT_A_BLOCK){
					dataBlocks[temp->getBlockAddress(j)]++;
					printf("Block %d for inode %d is allocated at address %d.\n", j, i, temp->getBlockAddress(j));
				}
		}

	}

	BitBlock *freeList;
	for (int i = 0; i < end; i++){
		freeList = fs->getFreeList(i);
		if (freeList->isBitSet(i) == true){
			printf("Address %d is allocated according to free list.\n", i);
		} else{
			printf("Address %d is free according to free list.\n", i);
		}
	}



    Kernel::Exit(0);

}

