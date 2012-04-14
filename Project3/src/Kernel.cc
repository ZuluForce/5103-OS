/* Simulates a unix-like file system.  Provides basic directory
   and file operations and implements them in terms of the underlying
   disk block structures.
*/

#include <io_types.h>
#include <FileDescriptor.h>
#include <Kernel.h>


ProcessContext *Kernel::process;
const String Kernel::PROGRAM_NAME  = "Kernel"; // name for error msgs
int Kernel::processCount;
FileDescriptor *Kernel::openFiles[MaxOpenFiles];
FileSystem *Kernel::openFileSystems [MaxOpenFileSystems];
int Kernel::MAX_OPEN_FILES;
const StringArr Kernel::sys_errlist =
	  {
	    null
	 , "Not owner"
	 , "No such file or directory"
	 , null
	 , null
	 , null
	 , null
	 , null
	 , null
	 , "Bad file number"
	 , "Expected non-NULL parameters"
	 , null
	 , null
	 , "Permission denied"
	 , null
	 , null
	 , null
	 , "File exists"
	 , "Cross-device link"
	 , null
	 , "Not a directory"
	 , "Is a directory"
	 , "Invalid argument"
	 , "File table overflow"
	 , "Too many open files"
	 , null
	 , null
	 , "File too large"
	 , "No space left on device"
	 , null
	 , "Read-only file system"
	 , "Too many links"
	 , "Link is broken"
	  };

int Kernel::MAX_OPEN_FILE_SYSTEMS;
short Kernel::ROOT_FILE_SYSTEM = 0;

void Kernel::perror(String s) {
  const char *message = null;
  if ((Kernel::process->errno > 0) && (Kernel::process->errno < Kernel::sys_nerr))
    message = Kernel::sys_errlist[Kernel::process->errno];
  if (message == null)
    fprintf (stderr, "%s%s%d\n", s," + : unknown errno ",Kernel::process->errno);
  else
    fprintf (stderr, "%s%s%s\n", s,":",  message);
}


void Kernel::setErrno(int newErrno) {
  if(Kernel::process == null) {
      fprintf (stderr, "no current process in setErrno\n");
      exit (Kernel::EXIT_F);
    }
  Kernel::process->errno = newErrno;
}

int Kernel::getErrno() {
    if(Kernel::process == null) {
      fprintf (stderr, "no current process in getErrno\n");
      exit (Kernel::EXIT_F);
    }
    return Kernel::process->errno;
}


int Kernel::close(int fd) {
  // check fd
  int status = check_fd(fd);
  if(status < 0)
    return status;

	short refInodeNum = process->openFiles[fd]->getIndexNodeNumber();
	IndexNode *refInode = process->openFiles[fd]->getIndexNode();

	// remove the file descriptor from the kernel's list of open files
	for(int i = 0; i < Kernel::MAX_OPEN_FILES; i ++) {

		if(openFiles[i] == process->openFiles[fd]) {
			if ( refInode->getNlink() == 0 ) {
				FileSystem *fs = openFileSystems[ROOT_FILE_SYSTEM];
				fs->freeIndexNode(refInodeNum);
			}

			openFiles[i] = null;
			break;
		}
	}
  // ??? is it an error if we didn't find the open file?

  // remove the file descriptor from the list.
  process->openFiles[fd] = null;
  process->num_files--;
  return 0;
}

int Kernel::creat(String pathname, short mode)
// throws Exception
{
  // get the full path
  String fullPath = getFullPath(pathname);

  StringBuffer *dirname = new StringBuffer("/");
  FileSystem *fileSystem = openFileSystems[ROOT_FILE_SYSTEM];
  IndexNode *currIndexNode = getRootIndexNode();
  IndexNode *prevIndexNode = null;
  short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

  StringTokenizer *st = new StringTokenizer(fullPath, "/");
  String name = "."; // start at root node
	while(st->hasMoreTokens()) {
		name = st->nextToken();
		if (strcmp(name, "")) {
			// check to see if the current node is a directory
			if((currIndexNode->getMode() & S_IFMT) != S_IFDIR)
			{
			  // return (ENOTDIR) if a needed directory is not a directory
			  process->errno = ENOTDIR;
			  return -1;
			}

			// check to see if it is readable by the user
			// ??? tbd
			// return (EACCES) if a needed directory is not readable

			if(st->hasMoreTokens()) {
			  dirname->append(name);
			  dirname->append('/');
			}

			// get the next inode corresponding to the token
			prevIndexNode = currIndexNode;
			currIndexNode = new IndexNode();
			indexNodeNumber = findNextIndexNode
			(fileSystem, prevIndexNode, name, currIndexNode);
		}
	}

  // ??? we need to set some fields in the file descriptor
  int flags = O_WRONLY; // ???
  FileDescriptor *fileDescriptor = null;

  if (indexNodeNumber < 0) {
		// file does not exist.  We check to see if we can create it.
		// check to see if the prevIndexNode (a directory) is writeable
		// ??? tbd
		// return (EACCES) if the file does not exist and the directory
		// in which it is to be created is not writable

		currIndexNode->setMode(mode);
		currIndexNode->setNlink((short)1);

		// allocate the next available inode from the file system
		short newInode = fileSystem->allocateIndexNode();
		if(newInode == -1)
			return -1;

		fileDescriptor =
		new FileDescriptor(fileSystem, currIndexNode, flags);
		// assign inode for the new file
		fileDescriptor->setIndexNodeNumber(newInode);

		fileSystem->writeIndexNode(currIndexNode, newInode);

		// open the directory
		// ??? it would be nice if we had an "open" that took an inode
		// instead of a name for the dir

		int dir = open(dirname->toString(), O_RDWR);
		if(dir < 0) {
			perror(PROGRAM_NAME);
			exit(1);
		}

		// scan past the directory entries less than the current entry
		// and insert the new element immediately following
		int status;
		DirectoryEntry *newDirectoryEntry = new DirectoryEntry(newInode, name);
		DirectoryEntry* currentDirectoryEntry = new DirectoryEntry();
		while (true) {
			// read an entry from the directory
			status = readdir(dir, currentDirectoryEntry);
			if (status < 0) {
			  fprintf (stderr, "error reading directory in creat\n");
			  exit(Kernel::EXIT_F);
			}
			else if (status == 0) {
			  // if no entry read, write the new item at the current
			  // location and break
			  writedir(dir, newDirectoryEntry);
			  break;
			}
			else {
				// if current item > new item, write the new item in
				// place of the old one and break
				if(strcmp (currentDirectoryEntry->getName(),
					newDirectoryEntry->getName()) > 0) {

					int seek_status = lseek(dir, - DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
					if (seek_status < 0) {
						fprintf (stderr, ": error during seek in creat\n");
						exit(Kernel::EXIT_F);
					}

				writedir(dir, newDirectoryEntry);
				break;
				}
			}
		}
		// copy the rest of the directory entries out to the file
		while (status > 0) {
			DirectoryEntry *nextDirectoryEntry = new DirectoryEntry();
			// read next item
			status = readdir(dir, nextDirectoryEntry);
			if(status > 0) {
		  		// in its place
				int seek_status =
				lseek(dir, - DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
				if(seek_status < 0) {
					fprintf (stderr, ": error during seek in creat\n");
					exit(Kernel::EXIT_F);
				}
			}

			// write current item
			writedir(dir, currentDirectoryEntry);
			// current item = next item
			currentDirectoryEntry = nextDirectoryEntry;
		}

      // close the directory
      close(dir);
    }
  else
    {
      // file does exist (indexNodeNumber >= 0)

      // if it's a directory, we can't truncate it
      if((currIndexNode->getMode() & S_IFMT) == S_IFDIR)
	{
	  // return (EISDIR) if the file is a directory
	  process->errno = EISDIR;
	  return -1;
	}

      // check to see if the file is writeable by the user
      // ??? tbd
      // return (EACCES) if the file does exist and is unwritable

      // free any blocks currently allocated to the file
      int blockSize = fileSystem->getBlockSize();
      int blocks = (currIndexNode->getSize() + blockSize - 1)/blockSize;
      for(int i = 0; i < blocks; i ++)
	{
	  int address = currIndexNode->getBlockAddress(i);
	  if(address != FileSystem::NOT_A_BLOCK)
	    {
	      fileSystem->freeBlock(address);
	      currIndexNode->setBlockAddress(i, FileSystem::NOT_A_BLOCK);
	    }
	}

      // update the inode to size 0
      currIndexNode->setSize(0);

      // write the inode to the file system.
      fileSystem->writeIndexNode(currIndexNode, indexNodeNumber);

      // set up the file descriptor
      fileDescriptor =
        new FileDescriptor(fileSystem, currIndexNode, flags);
      // assign inode for the new file
      fileDescriptor->setIndexNodeNumber(indexNodeNumber);
    }

  return open(fileDescriptor);
}


void Kernel::Exit(int status)
//  throws Exception
{
	// close anything that might be open for the current process
	for(int i = 0; i < process->num_files; i ++)
		if (Kernel::process->openFiles[i] != null) {
			close(i);
		}

  // terminate the process
  Kernel::process = null;
  Kernel::processCount --;

  // if this is the last process to end, call finalize
  if(Kernel::processCount <= 0)
    finalize(status);
}

int Kernel::lseek(int fd, int offset, int whence) {
	// check fd
	int status = check_fd(fd);
	if(status < 0)
		return status;

	FileDescriptor *file = process->openFiles[fd];

	int newOffset;
	if(whence == 0)
		newOffset = offset;
	else if(whence == 1)
		newOffset = file->getOffset() + offset;
	else if (whence == 2)
		newOffset = file->getSize() + offset;
	else {
		// bad whence value
		process->errno = EINVAL;
		return -1;
	}

	if(newOffset < 0) {
		// bad offset value
		process->errno = EINVAL;
		return -1;
	}

	file->setOffset(newOffset);
	return newOffset;
}

int Kernel::open(String pathname, int flags)
  // throws Exception
{
  // get the full path name
  String fullPath = getFullPath(pathname);

  IndexNode *indexNode = new IndexNode();
  short indexNodeNumber = findIndexNode(fullPath, indexNode);
  if(indexNodeNumber < 0)
    return -1;

  // ??? return (Exxx) if the file is not readable
  // and was opened O_RDONLY or O_RDWR

  // ??? return (Exxx) if the file is not writable
  // and was opened O_WRONLY or O_RDWR

  // set up the file descriptor
  FileDescriptor *fileDescriptor = new FileDescriptor
    (openFileSystems[ ROOT_FILE_SYSTEM ], indexNode, flags);
  fileDescriptor->setIndexNodeNumber(indexNodeNumber);

  return open(fileDescriptor);
}

int Kernel::open(FileDescriptor *fileDescriptor)
{
  // scan the kernel open file list for a slot
  // and add our new file descriptor
  int kfd = -1;
  for(int i = 0; i < Kernel::MAX_OPEN_FILES; i ++)
    if(openFiles[i] == null)
      {
        kfd = i;
        openFiles[kfd] = fileDescriptor;
        break;
      }
  if(kfd == -1)
    {
      // return (ENFILE) if there are already too many open files
      process->errno = ENFILE;
      return -1;
    }

  // scan the list of open files for a slot
  // and add our new file descriptor
  int fd = -1;
  for(int i = 0; i < ProcessContext::MAX_OPEN_FILES; i ++)
    if(process->openFiles[i] == null)
      {
        fd = i;
        process->openFiles[fd] = fileDescriptor;
	process->num_files++;
        break;
      }
  if(fd == -1)
    {
      // remove the file from the kernel list
      openFiles[kfd] = null;
      // return (EMFILE) if there isn't room left
      process->errno = EMFILE;
      return -1;
    }

  // return the index of the file descriptor for now open file
  return fd;
}

int Kernel::fdOpen(int fd) {
	if ( fd >= ProcessContext::MAX_OPEN_FILES ) {
		Kernel::setErrno(EBADF);
		return -1;
	}

	FileDescriptor *openFile = process->openFiles[fd];
	if ( openFile == NULL ) {
		Kernel::setErrno(EBADF);
		return -1;
	}

	FileDescriptor *newFile =
	new FileDescriptor(openFileSystems[ROOT_FILE_SYSTEM],
						openFile->getIndexNode(),
						openFile->getFlags());

	newFile->setIndexNodeNumber(openFile->getIndexNodeNumber());

	int newfd = -1;
	newfd = open(newFile);
	if ( newfd < 0 )
		return newfd;

	lseek(newfd, openFile->getOffset(),0);
	return newfd;
}


//Just fo debugging purposes
void Kernel::printOffsets(int fd1, int fd2) {
	FileDescriptor *f1 = process->openFiles[fd1];
	FileDescriptor *f2 = process->openFiles[fd2];

	fprintf(stderr, "f1 off: %d   f2 off: %d\n", f1->getOffset(), f2->getOffset());
}

bool Kernel::isOpen(short nodeNum) {
	for (int i = 0; i < MAX_OPEN_FILES; ++i) {
		if ( openFiles[i] &&
			openFiles[i]->getIndexNodeNumber() == nodeNum )
			return true;
	}

	return false;
}

bool Kernel::isOpen(IndexNode* node) {
	for (int i = 0; i < MAX_OPEN_FILES; ++i) {
		if ( openFiles[i] &&
			openFiles[i]->getIndexNode() == node )
				return true;
	}

	return false;
}


int Kernel::read(int fd, byte *buf, int count) {
  // check fd
  int status = check_fd_for_read(fd);
  if(status < 0)
    return status;

  FileDescriptor *file = process->openFiles[fd];
  int offset = file->getOffset();
  int size = file->getSize();
  int blockSize = file->getBlockSize();
  byte* bytes = file->getBytes();
  int readCount = 0;
  for(int i = 0; i < count; i ++)
    {
      // if we read to the end of the file, stop reading
      if(offset >= size)
        break;
      // if this is the first time through the loop,
      // or if we're at the beginning of a block, load the data block
      if((i == 0) || ((offset % blockSize) == 0))
	{
	  status = file->readBlock((short)(offset / blockSize));
	  if(status < 0)
	    return status;
	}
      // copy a byte from the file buffer to the read buffer
      buf[i] = bytes[ offset % blockSize ];
      offset ++;
      readCount ++;
    }
    // update the offset
    file->setOffset(offset);

    // return the count of bytes read
    return readCount;
  }

int Kernel::readdir(int fd, DirectoryEntry *dirp) {
  // check fd
  int status = check_fd_for_read(fd);
  if(status < 0)
    return status;

  FileDescriptor *file = process->openFiles[fd];

  // check to see if the file is a directory
  if((file->getMode() & S_IFMT) != S_IFDIR)
    {
      // return (ENOTDR) if a needed directory is not a directory
      process->errno = ENOTDIR;
      return -1;
    }

  // return 0 if at end of directory
  if(file->getOffset() >= file->getSize())
    return 0;

  // read a block, if needed
    status = file->readBlock((short)(file->getOffset() / file->getBlockSize()));
    if(status < 0)
      return status;

    // read bytes from the block into the DirectoryEntry
    dirp->read(file->getBytes(),
      file->getOffset() % file->getBlockSize());
    file->setOffset(file->getOffset() +
      DirectoryEntry::DIRECTORY_ENTRY_SIZE);

    // return the size of a DirectoryEntry
    return DirectoryEntry::DIRECTORY_ENTRY_SIZE;
}

int Kernel::changeSize(int fd, int newsize, int how) {
	//if ( check_fd(fd) < 0 ) {
	//	Kernel::setErrno(EBADF);
	//	return -1;
	//}

	FileDescriptor *file = process->openFiles[fd];

	int status = 0;
	int realsize = 0;
	if ( how == 0 ) {
		realsize = newsize;
	} else if ( how == 1 ) {
		realsize = file->getSize() + newsize;
	} else {
		Kernel::setErrno(EINVAL);
		return -1;
	}

	//Check if the change in size crossed a block boundary
	int blockSize = file->getBlockSize();
	int currBlocks = ((file->getSize() - 1) / blockSize) + 1;
	int newBlocks = ((realsize - 1) / blockSize) + 1;
	currBlocks = currBlocks < 0 ? 0 : currBlocks;
	newBlocks = newBlocks < 0 ? 0 : newBlocks; //In-case the size is being set to 0
	//size - 1 so it wraps to the next block at blockSize + 1 bytes.
	if ( newBlocks < currBlocks) {
		//Deallocate the unneeded blocks
		FileSystem *fs = openFileSystems[ROOT_FILE_SYSTEM];
		status = fs->freeInodeBlocks(file->getIndexNodeNumber(),
							currBlocks - newBlocks);

		if (status < 0) return status;
	}

	file->setSize(realsize);

	return 0;
}

int Kernel::fstat(int fd, Stat *buf) {
	// check fd
	int status = check_fd(fd);
	if(status < 0)
		return status;

	FileDescriptor *fileDescriptor = process->openFiles[fd];
	short deviceNumber = fileDescriptor->getDeviceNumber();
	short indexNodeNumber = fileDescriptor->getIndexNodeNumber();
	IndexNode *indexNode = fileDescriptor->getIndexNode();

	// copy information to buf
	buf->st_dev = deviceNumber;
	buf->st_ino = indexNodeNumber;
	buf->copyIndexNode(indexNode);

	return 0;
}

int Kernel::stat(String name, Stat *buf, bool leaveLink) {
	// a buffer for reading directory entries
	DirectoryEntry *directoryEntry = new DirectoryEntry();

	// get the full path
	String path = getFullPath(name);

	// find the index node
	IndexNode *indexNode = new IndexNode();
	short indexNodeNumber = findIndexNode(path, indexNode, leaveLink);
	if(indexNodeNumber < 0) {

		if ( process->errno == EBLINK )
			return -1;

		// return ENOENT
		process->errno = ENOENT;
		return -1;
	}

	// copy information to buf
	buf->st_dev = ROOT_FILE_SYSTEM;
	buf->st_ino = indexNodeNumber;
	buf->copyIndexNode(indexNode);

	return 0;
}

void Kernel::sync() {
  // write out superblock if updated
  // write out free list blocks if updated
  // write out inode blocks if updated
  // write out data blocks if updated

  // at present, all changes to inodes, data blocks,
  // and free list blocks
  // are written as they go, so this method does nothing.
}

int Kernel::write(int fd, byte *buf, int count)
// throws Exception
{
  // check fd
  int status = check_fd_for_write(fd);
  if(status < 0)
    return status;

  FileDescriptor *file = process->openFiles[fd];

  // return (ENOSPC) if the device containing the file system
  // referred to by fd has not room for the data

  int offset = file->getOffset();
  int size = file->getSize();
  int blockSize = file->getBlockSize();
  byte* bytes = file->getBytes();
  int writeCount = 0;
  for(int i = 0; i < count; i ++)
    {
      // if this is the first time through the loop,
      // or if we're at the beginning of a block,
      // load or allocate a data block
      if((i == 0) || ((offset % blockSize) == 0))
      {
        status = file->readBlock((short)(offset / blockSize));
        if(status < 0)
          return status;
      }
      // copy a byte from the write buffer to the file buffer
      bytes[ offset % blockSize ] = buf[i];
      offset ++;
      // if we get to the end of a block, write it out
      if((offset % blockSize) == 0)
	{
	  status =
	    file->writeBlock((short)((offset - 1) / blockSize));
	  if(status < 0)
	    return status;
	  // update the file size if it grew
	  if(offset > size)
	    {
	      file->setSize(offset);
	      size = offset;
	    }
	}
      writeCount ++;
    }

  // write the last block if we wrote anything to it
  if((offset % blockSize) > 0)
    {
      status = file->writeBlock((short)((offset - 1) / blockSize));
      if(status < 0)
        return status;
    }

  // update the file size if it grew
  if(offset > size)
    file->setSize(offset);

  // update the offset
  file->setOffset(offset);

  // return the count of bytes written
  return writeCount;
}

int Kernel::writedir(int fd, DirectoryEntry *dirp)
// throws Exception
{
  // check fd
  int status = check_fd_for_write(fd);
  if(status < 0)
    return status;

  FileDescriptor *file = process->openFiles[fd];

  // check to see if the file is a directory
  if((file->getMode() & S_IFMT) != S_IFDIR)
    {
      // return (ENOTDIR) if a needed directory is not a directory
      process->errno = ENOTDIR;
      return -1;
    }

  short blockSize = file->getBlockSize();
  // allocate or read a block
  status = file->readBlock((short)(file->getOffset() / blockSize));
  if(status < 0)
    return status;

  // write bytes from the DirectoryEntry into the block
  dirp->write(file->getBytes(), file->getOffset() % blockSize);

  // write the updated block
  status = file->writeBlock((short)(file->getOffset() / blockSize));
  if(status < 0)
    return status;

  // update the file size
  file->setOffset(file->getOffset() +
		  DirectoryEntry::DIRECTORY_ENTRY_SIZE);
  if(file->getOffset() > file->getSize())
    file->setSize(file->getOffset());

  // return the size of a DirectoryEntry
  return DirectoryEntry::DIRECTORY_ENTRY_SIZE;
}

/*
to be done:
       int access(const char *pathname, int mode);
       int link(const char *oldpath, const char *newpath);
       int unlink(const char *pathname);
       int rename(const char *oldpath, const char *newpath);
       int symlink(const char *oldpath, const char *newpath);
       int lstat(const char *file_name, struct stat *buf);
       int chmod(const char *path, mode_t mode);
       int fchmod(int fildes, mode_t mode);
       int chown(const char *path, uid_t owner, gid_t group);
       int fchown(int fd, uid_t owner, gid_t group);
       int lchown(const char *path, uid_t owner, gid_t group);
       int utime(const char *filename, struct utimbuf *buf);
       int readlink(const char *path, char *buf, size_t bufsiz);
       int chdir(const char *path);
       mode_t umask(mode_t mask);
*/

void Kernel::initialize() {
	// We are hard-setting the properties ... could also check filesys.conf
	// get the root file system properties
	String rootFileSystemFilename = "filesys.dat";
	String rootFileSystemMode = "rw";
	short uid = 1;
	uid = 1;
	short gid = 1;
	short umask = 0002;
	String dir = "/root";
	MAX_OPEN_FILES = 20;
	ProcessContext::MAX_OPEN_FILES = 10;

	// create open file array
	for (int i=0; i<MAX_OPEN_FILES; i++) {
		openFiles[i] = new FileDescriptor;
		openFiles[i] = null;
	}

	// create the first process
	Kernel::process = new ProcessContext(uid, gid, dir, umask);
	Kernel::processCount++;

	// open the root file system -- should catch error if it fails!
	openFileSystems[ROOT_FILE_SYSTEM] =
	  new FileSystem(rootFileSystemFilename, rootFileSystemMode);
}


void Kernel::finalize(int status) {
	// exit() any remaining processes
	if(Kernel::process != null)
		Exit(0);

	// flush file system blocks
	sync();

	// close the root file system
	openFileSystems[ROOT_FILE_SYSTEM]->close();

	// terminate the program
	exit (status);
}

/* Some internal methods. */

int Kernel::check_fd(int fd) {
  // look for the file descriptor in the open file list
  if (fd < 0 ||
      fd >= Kernel::process->num_files ||
      Kernel::process->openFiles[fd] == null)
    {
      // return (EBADF) if file descriptor is invalid
      Kernel::process->errno = EBADF;
      return -1;
    }

  return 0;
}

int Kernel::check_fd_for_read(int fd) {
	int status = check_fd(fd);
	if(status < 0)
	return -1;

	FileDescriptor *fileDescriptor = Kernel::process->openFiles[fd];
	int flags = fileDescriptor->getFlags();
	if((flags != O_RDONLY) && (flags != O_RDWR)) {
		// return (EBADF) if the file is not open for reading
		Kernel::process->errno = EBADF;
		return -1;
	}

	return 0;
}

int Kernel::check_fd_for_write(int fd) {
	int status = check_fd(fd);
	if(status < 0)
		return -1;

	FileDescriptor *fileDescriptor = Kernel::process->openFiles[fd];
	int flags = fileDescriptor->getFlags();
	if((flags != O_WRONLY) && (flags != O_RDWR)){
	  // return (EBADF) if the file is not open for writing
	  Kernel::process->errno = EBADF;
	  return -1;
	}

	return 0;
}

String Kernel::getFullPath(String pathname) {
	String fullPath = null;

	// make sure the path starts with a slash
	//    if(pathname.startsWith("/"))
	if(pathname[0] ==  '/')
		fullPath = pathname;
	else {
		char *temp = new char[100]; // should clean up ... use StringBuffer
		strcpy (temp, Kernel::process->getDir());
		strcat (temp, "/");
		fullPath = strcat (temp, pathname);
	}

	return fullPath;
}

String Kernel::getDeepestDir(String pathname, bool ignoreTrail) {
	if ( ignoreTrail && pathname[strlen(pathname) - 1] == '/')
		pathname[strlen(pathname)-1] = '\0';

	StringBuffer *path = new StringBuffer("/");

	StringTokenizer *st = new StringTokenizer(pathname, "/");

	String name = ".";
	while (st->hasMoreTokens()) {
		name = st->nextToken();

		if ( !st->hasMoreTokens() ) {
			//Throw out the last part
			break;
		}

		path->append(name);
		path->append('/');
	}

	return path->toString();
}

IndexNode *Kernel::rootIndexNode = null;
IndexNode *Kernel::getRootIndexNode() {
	if(rootIndexNode == null)
		rootIndexNode = openFileSystems[ROOT_FILE_SYSTEM]->getRootIndexNode();

	return rootIndexNode;
}

short Kernel::findNextIndexNode
(FileSystem *fileSystem, IndexNode *indexNode, String name,
 IndexNode *nextIndexNode, bool leaveLink) {
	// if stat isn't a directory give an error
	if((indexNode->getMode() & S_IFMT) != S_IFDIR) {
		// return (ENOTDIR) if a needed directory is not a directory
		Kernel::process->errno = ENOTDIR;
		return -1;
	}

  // if user isn't alowed to read directory, give an error
  // ??? tbd
  // return (EACCES) if a needed directory is not readable
  FileDescriptor *fileDescriptor =
    new FileDescriptor(fileSystem, indexNode, O_RDONLY);
	int fd = open(fileDescriptor);
	if(fd < 0) {
		// process->errno = ???
		return -1;
	}

  // create a buffer for reading directory entries
  DirectoryEntry *directoryEntry = new DirectoryEntry();

  int status = 0;
  int close_status;
  short indexNodeNumber = -1;
  // while there are more directory blocks to be read
  while(true) {
      // read a directory entry
      status = readdir(fd, directoryEntry);
      if(status <= 0) {
        // we got to the end of the directory, or
        // encountered an error, so quit
        break;
      }

      if(!strcmp (directoryEntry->getName(), name))
      {
        indexNodeNumber = directoryEntry->getIno();
        // read the inode block
        fileSystem->readIndexNode(nextIndexNode, indexNodeNumber);
        // we're done searching...Oh wait...
        //It could be a symlink!!
        if ((nextIndexNode->getMode() & S_IFMT) == S_IFSYM) {

			if ( !leaveLink ) {
				indexNodeNumber = resolveSymlinkNode(fileSystem,
													nextIndexNode, nextIndexNode);

				if ( indexNodeNumber < 0 ) {
					Kernel::setErrno(EBLINK);
					status = -1;
				}
			}

			break;
        }

        break;
      }
	}

    // close the file since we're done with it
    close_status = close(fd);
    if(close_status < 0) {
      // process->errno = ???
      return -1;
    }

    // if we encountered an error reading, return error
    if(status < 0) {
      // process->errno = ???
      return -1;
    }

    // if we got to the directory without finding the name, return error
    if(status == 0) {
      Kernel::process->errno = ENOENT;
      return -1;
    }

    // return index node number if success
    return indexNodeNumber;
}

short Kernel::resolveSymlinkNode(FileSystem *fs, IndexNode *inode,
								IndexNode *resolveInode) {

	if ( (inode->getMode() & S_IFMT) != S_IFSYM ) {
		Kernel::setErrno(EINVAL);
		return -1;
	}

	int status = 0;
	//Get address out of symlink
	FileDescriptor *fd = new FileDescriptor(fs, inode, O_RDONLY);
	int _fd = open(fd);
	if ( _fd < 0 ) {
		delete fd;
		return -1;
	}

	byte path[inode->getSize()];
	memset((void*) path, '\0', inode->getSize());
	status = Kernel::read(_fd, path, inode->getSize());
	path[inode->getSize()] = '\0';

	close(_fd);
	delete fd;

	if ( status < 0 )
		return status;

	status = findIndexNode((String) path, resolveInode);

	return status;
}

// get the inode for a file which is expected to exist
short Kernel::findIndexNode(String path, IndexNode *inode, bool leaveLink) {
	// start with the root file system, root inode
	FileSystem *fileSystem = openFileSystems[ ROOT_FILE_SYSTEM ];
	IndexNode *indexNode = getRootIndexNode();
	short indexNodeNumber = FileSystem::ROOT_INDEX_NODE_NUMBER;

	bool lastNode = false;
  // parse the path until we get to the end
  StringTokenizer_rr *st = new StringTokenizer_rr(path, "/");
  while(st->hasMoreTokens()) {
      String s = st->nextToken();

      if (strcmp (s,"")) {
        // check to see if it is a directory
        if((indexNode->getMode() & S_IFMT) != S_IFDIR)
        {
          // return (ENOTDIR) if a needed directory is not a directory
          Kernel::process->errno = ENOTDIR;
          return -1;
        }

        // check to see if it is readable by the user
        // ??? tbd
        // return (EACCES) if a needed directory is not readable
        IndexNode *nextIndexNode = new IndexNode();
        // get the next index node corresponding to the token
        lastNode = !st->hasMoreTokens() && leaveLink;
        indexNodeNumber = findNextIndexNode
	  (fileSystem, indexNode, s, nextIndexNode, lastNode);

        if(indexNodeNumber < 0) {
			if ( process->errno == EBLINK )
				return -1;
          // return ENOENT
          Kernel::process->errno = ENOENT;
          return -1;
        }
        indexNode = nextIndexNode;
      }
    }
	// copy indexNode to inode
	indexNode->copy(inode);

	//Cleanup
	delete st;

	return indexNodeNumber;
}

int Kernel::updateIndexNode(IndexNode *node, short nodenum) {
	if ( node == NULL || nodenum < 0 ) {
		Kernel::setErrno(EINVAL);
		return -1;
	}

	FileSystem *fs = openFileSystems[ROOT_FILE_SYSTEM];

	fs->writeIndexNode(node, nodenum);
	return 0;
}


int Kernel::link(String oldpath, String newPath) {
	if ( oldpath == NULL || newPath == NULL ) {
		//Should set errno here
		Kernel::setErrno(ENULL);
		return -1;
	}

	int status = 0;

	//Get inode of oldpath
	IndexNode *oldinode = new IndexNode();
	short node_num = Kernel::findIndexNode(oldpath, oldinode);
	fprintf(stderr, "Got inode to link to (%d)\n", node_num);
	if ( node_num < 0 ) {
		return status;
	}

	String dirname = Kernel::getDeepestDir(newPath);
	fprintf(stderr, "Directory name: %s\n", dirname);
	StringBuffer *fname = new StringBuffer("");
	StringCut(newPath, dirname, fname); //Get just the filename
	fprintf(stderr, "New filename: %s\n", fname->toString());

	if ( fname->toString() == "" ) {
		//Trying to create a link as a directory
		Kernel::setErrno(EISDIR);
		return -1;
	}

	int dir = open(dirname, O_RDWR);
	if (dir < 0) {
		perror(PROGRAM_NAME);
		return -1;
	}

	DirectoryEntry* currDirEntry = new DirectoryEntry();
	DirectoryEntry* newDirEntry = new DirectoryEntry(node_num, fname->toString());

	int cmpStatus = 0;
	while (true) {
		status = readdir(dir, currDirEntry);
		if (status < 0) {
			fprintf(stderr, "error reading directory in link\n");
			exit(Kernel::EXIT_F);
		} else if (status == 0) {
			//Directory empty, go ahead an make new entry
			writedir(dir, newDirEntry);
			break;
		} else {
			//Since directories are kept sorted we need to check
			//its position
			cmpStatus = strcmp(currDirEntry->getName(),newDirEntry->getName());
			if ( cmpStatus > 0) {
				int seek_status = lseek(dir, - DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
				if (seek_status < 0) {
					fprintf(stderr, ": error during seek in link\n");
					exit(Kernel::EXIT_F);
				}

				writedir(dir, newDirEntry);
				break;
			} else if ( cmpStatus == 0 ) {
				Kernel::setErrno(EEXIST);
				return -1;
			}
		}
	}

	while (status > 0) {
		DirectoryEntry *nextDirEntry = new DirectoryEntry();

		status = readdir(dir, nextDirEntry);
		if (status > 0)	{
			int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
			if (seek_status < 0) {
				fprintf(stderr, ": error during seek in link\n");
				exit(Kernel::EXIT_F);
			}
		}

		writedir(dir, currDirEntry);
		currDirEntry = nextDirEntry;
	}

	close(dir);

	oldinode->incNlink();
	updateIndexNode(oldinode, node_num);

	delete fname;

	return 0;
}

int Kernel::unlink(String pathname) {
	String dirname = Kernel::getDeepestDir(pathname);
	StringBuffer *_fname = new StringBuffer("");
	StringCut(pathname,dirname,_fname);
	String fname = _fname->toString();

	delete _fname;

	int dirfd = open(dirname, O_RDWR);

	if ( dirfd < 0 ) {
		Kernel::setErrno(ENOENT);
		return -1;
	}

	//Now find the filename in the directory
	int status = 1;
	bool deleted = false;
	DirectoryEntry *currEntry = new DirectoryEntry();
	while (status > 0) {
		status = readdir(dirfd, currEntry);
		if ( status < 0 ) {
			fprintf(stderr, "error reading directory in unlink\n");
			exit(Kernel::EXIT_F);
		}
		if (!strcmp(currEntry->getName(),fname)) {
			fprintf(stderr, "Found matching directory entry\n");
			//Found the directory entry
			IndexNode* refInode = new IndexNode();
			short refInodeNum = 0;
			refInodeNum = findIndexNode(pathname,refInode);
			if ( refInodeNum < 0 ) {
				perror(PROGRAM_NAME);
				return -1;
			}

			refInode->decNlink();
			if ( refInode->getNlink() == 0 &&
				 !Kernel::isOpen(refInode)) {
				//Not open and this was the last link...remove it
				FileSystem *fs = openFileSystems[ROOT_FILE_SYSTEM];
				fs->freeIndexNode(currEntry->getIno());
			}

			//Remove the directory entry

			/* I'm openinng a new file descriptor so it will have
			 * a different seek position to the original one. This is
			 * used to copy up any directory entries below the removed
			 * one */
			int fd2 = fdOpen(dirfd);
			if ( fd2 < 0 )
				return fd2;
			lseek(fd2, -DirectoryEntry::DIRECTORY_ENTRY_SIZE,1);

			status = readdir(dirfd, currEntry);
			while (status > 0) {
				printOffsets(dirfd, fd2);
				writedir(fd2, currEntry);

				status = readdir(dirfd, currEntry);
			}

			if ( status < 0)
				return status;

			status = changeSize(fd2,-DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
			if ( status < 0 )
				return status;

			close(dirfd);
			close(fd2);

			deleted = true;
			break;
		}
	}

	if ( !deleted ) {
		Kernel::setErrno(ENOENT);
		return -1;
	}

	return 0;
}

int Kernel::symlink(String oldpath, String newpath) {
	if ( oldpath == NULL || newpath == NULL ) {
		//Should set errno here
		Kernel::setErrno(ENULL);
		return -1;
	}

	//First check that the target file/directory exists
	IndexNode *checkNode = new IndexNode();
	int targetNode = findIndexNode(oldpath, checkNode, false);
	delete checkNode;

	if ( targetNode < 0 ) {
		Kernel::setErrno(ENOENT);
		return -1;
	}

	int status = 0;

	//We are going to need an inode
	FileSystem *fs = openFileSystems[ROOT_FILE_SYSTEM];
	FileDescriptor *newFd = NULL;
	short newInodeNum = fs->allocateIndexNode();
	if ( newInodeNum == -1)
		return -1;

	IndexNode *inode = new IndexNode();
	inode->setMode(S_IFSYM);
	inode->setNlink((short)1);
	newFd = new FileDescriptor(fs, inode, O_WRONLY);
	int _newFd = open(newFd);

	newFd->setIndexNodeNumber(newInodeNum);
	fs->writeIndexNode(inode, newInodeNum);

	//Write the pathname into the symlink
	Kernel::write(_newFd, (byte*) oldpath, strlen(oldpath)+1);

	IndexNode *temp = new IndexNode();
	fs->readIndexNode(temp, newInodeNum);

	close(_newFd);
	delete newFd;

	//Below here it is almost the same as link
	String dirname = Kernel::getDeepestDir(newpath);
	fprintf(stderr, "Directory name: %s\n", dirname);
	StringBuffer *fname = new StringBuffer("");
	StringCut(newpath, dirname, fname); //Get just the filename
	fprintf(stderr, "New filename: %s\n", fname->toString());

	if ( fname->toString() == "" ) {
		//Trying to create a link as a directory
		Kernel::setErrno(EISDIR);
		return -1;
	}

	int dir = open(dirname, O_RDWR);
	if (dir < 0) {
		perror(PROGRAM_NAME);
		return -1;
	}

	DirectoryEntry* currDirEntry = new DirectoryEntry();
	DirectoryEntry* newDirEntry = new DirectoryEntry(newInodeNum, fname->toString());
	fprintf(stdout, "New symlink name: %s\n", fname->toString());

	int cmpStatus = 0;
	while (true) {
		status = readdir(dir, currDirEntry);
		if (status < 0) {
			fprintf(stderr, "error reading directory in symlink\n");
			exit(Kernel::EXIT_F);
		} else if (status == 0) {
			//Directory empty, go ahead an make new entry
			writedir(dir, newDirEntry);
			break;
		} else {
			//Since directories are kept sorted we need to check
			//its position
			cmpStatus = strcmp(currDirEntry->getName(),newDirEntry->getName());
			if ( cmpStatus > 0) {
				int seek_status = lseek(dir, - DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
				if (seek_status < 0) {
					fprintf(stderr, ": error during seek in link\n");
					exit(Kernel::EXIT_F);
				}

				writedir(dir, newDirEntry);
				break;
			} else if ( cmpStatus == 0 ) {
				Kernel::setErrno(EEXIST);
				return -1;
			}
		}
	}

	while (status > 0) {
		DirectoryEntry *nextDirEntry = new DirectoryEntry();

		status = readdir(dir, nextDirEntry);
		if (status > 0)	{
			int seek_status = lseek(dir, -DirectoryEntry::DIRECTORY_ENTRY_SIZE, 1);
			if (seek_status < 0) {
				fprintf(stderr, ": error during seek in link\n");
				exit(Kernel::EXIT_F);
			}
		}

		writedir(dir, currDirEntry);
		currDirEntry = nextDirEntry;
	}

	close(dir);

	delete fname;

	return 0;
}

bool Kernel::validFileName(String name) {
	char *stayNull = NULL;

	stayNull = strrchr((char*) name, '/');
	if ( stayNull != NULL ) return false;

	stayNull = strstr((char*) name, "..");
	if ( stayNull != NULL ) return false;

	return true;
}


void Kernel::incIndexNodeNlink(int fd){
	FileDescriptor *fileD = process->openFiles[fd];
	IndexNode *node = fileD->getIndexNode();
	printf("node %d before:%d\n", fileD->getIndexNodeNumber(), node->getNlink());
	node->incNlink();
	printf("node %d after:%d\n", fileD->getIndexNodeNumber(), node->getNlink());
	updateIndexNode(node, fileD->getIndexNodeNumber());
}

int Kernel::filesysStatus(int fsn) {
	if ( fsn >= MaxOpenFileSystems ) {
		fprintf(stderr, "fsn = %d\n", fsn);
		Kernel::setErrno(EINVAL);
		return -1;
	}

	FileSystem *fs = openFileSystems[fsn];

	int totalBlocks = fs->getBlockCount();

	int takenInode = fs->getTakenInodes();
	int totalInode = fs->getInodeCount();

	int inodeBlocks = totalInode / (fs->getBlockSize() / IndexNode::INDEX_NODE_SIZE);

	int takenDBlocks = fs->getTakenDBlocks();
	int totalDBlocks = totalBlocks - inodeBlocks - 2;



	fprintf(stdout, "/* ---- Printing Status of Filesystem %d ---- */\n", fsn);
	fprintf(stdout, "Block Size: %d\n", fs->getBlockSize());
	fprintf(stdout, "Total Blocks: %d\n", totalBlocks);
	fprintf(stdout, "DBlocks Alloc'd: %d (total %d)\n", takenDBlocks, totalDBlocks);
	fprintf(stdout, "Inodes Allocated: %d (total %d in %d blocks)\n",
			takenInode, totalInode, inodeBlocks);
	fprintf(stdout, "/* ----------------------------------------- */\n");

}
