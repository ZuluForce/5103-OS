#include <Kernel.h>

static const String PROGRAM_NAME = "crpt";



/* crpt command */
int main(int argc, char **argv)
// throws Exception
{
	if(argc != 3)
	{
	      fprintf (stderr, "usage: crpt <num-inodes-> <num-blocks>\n");
	      exit(0);
	}

	int status = 0;

	int numNodes = atoi(argv[1]);
	int numNodesInside = numNodes / 2;
	int numNodesOutside = numNodes - numNodesInside;
	int numBlocks = atoi(argv[2]);
	int numBlocksAllocate = numBlocks / 2;
	int numBlocksDeallocate = numBlocks - numBlocksAllocate;

  // initialize the file system simulator kernel
    Kernel::initialize();
    FileSystem *fs = Kernel::openFileSystems[Kernel::ROOT_FILE_SYSTEM];


    int numINodes = fs->getInodeCount();
    if (numINodes < numNodes + 1){
    	fprintf(stderr, "%s: not enough IndexNodes to corrupt\n", PROGRAM_NAME);
    	Kernel::Exit(1);
    }

    int numDataBlocks = fs->getBlockCount() - fs->getDataBlockOffset();
    int numBlocksTaken = fs->getTakenDBlocks();

    if ((numDataBlocks - numBlocksTaken) < numBlocksAllocate){
    	fprintf(stderr, "%s: not enough free blocks to allocate\n", PROGRAM_NAME);
    	Kernel::Exit(1);
    }

    if (numBlocksTaken < numBlocksDeallocate){
    	fprintf(stderr, "%s: not enough taken blocks to deallocate\n", PROGRAM_NAME);
		Kernel::Exit(1);
    }

 	status = Kernel::corruptFileSys(numNodesInside, numNodesOutside, numBlocksAllocate, numBlocksDeallocate);
 	if (status < 0){
 		Kernel::perror(PROGRAM_NAME);
 		fprintf(stderr, "%s%s\n", PROGRAM_NAME, "Did not corrupt filesystem completely.");
 	}

    Kernel::Exit(0);

}

