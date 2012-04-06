/* Simulates a unix-like file system.  Provides basic directory
   and file operations and implements them in terms of the underlying
   disk block structures.
*/

#include <io_types.h>
#include <DirectoryEntry.h>
#include <Stat.h>
#include <ProcessContext.h>
#include <stdlib.h>
#include <FileDescriptor.h>

class Kernel {
public:
	static const String PROGRAM_NAME;  // name for error msgs

	// List of Errors ...

	static const int EPERM = 1; // not owner
	static const int ENOENT = 2; // No such file or directory.
	static const int EBADF = 9; // Bad file number.
	static const int ENULL = 10;
	static const int EACCES = 13; // Permission denied.
	static const int EEXIST = 17; // File exists.
	static const int EXDEV = 18; // Cross-device link.
	static const int ENOTDIR = 20; // Not a directory.
	static const int EISDIR = 21; // Is a directory.
	static const int EINVAL = 22; // Invalid argument.
	static const int ENFILE = 23; // File table overflow.
	static const int EMFILE = 24; // Too many open files.
	static const int EFBIG = 27; // File too large.
	static const int ENOSPC = 28; // No space left on device.
	static const int EROFS = 30; // Read-only file system.
	static const int EMLINK = 31; // Too many links.

	static const int sys_nerr = 32; // # of erros in sys_errlist

	static const StringArr sys_errlist;

	static void setErrno(int);
	static int getErrno();
	static void perror(String);

	/* Modes */
	// File type masks
	static const short S_IFMT = (short)0170000;
	static const short S_IFREG = (short)0100000; // Regular file
	static const short S_IFMPB = 070000; // Multiplexed block special
	static const short S_IFBLK = 060000; // Block Special
	static const short S_IFDIR = 040000; // Directory
	static const short S_IFMPC = 030000; // Multiplexed character special
	static const short S_IFCHR = 020000; // Character special
	static const short S_ISUID = 04000; // Set user id on execution
	static const short S_ISGID = 02000; // Set group id on execution
	static const short S_ISVTX = 01000; // Save swapped text even after use

	// User (file owner) has read, write and execute permission
	static const short S_IRWXU = 0700;
	static const short S_IRUSR = 0400; // User has read permission
	static const short S_IREAD = 0400; // User has read permission
	static const short S_IWUSR = 0200; // User has write permission
	static const short S_IWRITE = 0200; // User has write permission
	static const short S_IXUSR = 0100; // User has execute permission
	static const short S_IEXEC = 0100; //  User has execute permission
	static const short S_IRWXG = 070; // Group has read, write and execute permission
	static const short S_IRGRP = 040; // Group has read permission
	static const short S_IWGRP = 020; // Group has write permission
	static const short S_IXGRP = 010; // Group has execute permission

	// Others have read, write and execute permission
	static const short S_IRWXO = 07;
	static const short S_IROTH = 04; // Others have read permission
	static const short S_IWOTH = 02; // Others have write permisson
	static const short S_IXOTH = 01; // Others have execute permission

	// Returns 0 if the file is closed; -1 if the file descriptor
	// is invalid.
	static int close(int fd);

	/* Creates a file or directory with the specified mode.
	Creates a new file or prepares to rewrite an existing file.
	If the file does not exist, it is given the mode specified.
	If the file does exist, it is truncated to length zero.
	The file is opened for writing and its file descriptor is
	returned. Simulates the unix system call:
	*   int creat(const char *pathname, mode_t mode);
	pathname the name of the file or directory to create;
	mode the file or directory protection mode for the new file;
	return the file descriptor (a non-negative integer); -1 if
	a needed directory is not searchable, if the file does not
	exist and the directory in which it is to be created is not
	writable, if the file does exist and is unwritable, if the
	file is a directory, or if there are already too many open
	files.
	*/
	static int creat(String pathname, short mode);  // throws Exception

	/* Terminate the current "process".  Any open files will be closed.
	Simulates the unix system call:
	*   exit(int status);
	Note: If this is the last process to terminate, this method
	calls finalize().
	*/

	static void Exit(int status);  // throws Exception

	/* Set the current file pointer for a file.
	The current file position is updated based on the values of
	offset and whence.  If whence is 0, the new position is
	offset bytes from the beginning of the file.  If whence is
	1, the new position is the current position plus the value
	of offset.  If whence is 2, the new position is the size
	of the file plus the offset value.  Note that offset may be
	negative if whence is 1 or 2, as long as the resulting
	position is not less than zero.  It is valid to position
	past the end of the file, but it is not valid to read
	past the end of the file. Simulates the unix system call:
	*   lseek(int filedes, int offset, int whence);
	*/
	static int lseek(int fd, int offset, int whence);

	static const int O_RDONLY = 0; // Open with read-only access.
	static const int O_WRONLY = 1; // Open with write-only access.
	static const int O_RDWR = 2; // Open for read or write access.

	/*  Opens a file or directory for reading, writing, or
	  both reading and writing.
	  The file is positioned at the beginning (byte 0).
	  The returned file descriptor must be used for subsequent
	  calls for other input and output functions on the file.
	  Simulates the unix system call:
	*   int open(const char *pathname, int flags);
	pathname the name of the file or directory to create;
	flags is the flags to use when opening the file: O_RDONLY,
	O_WRONLY, or O_RDWR;
	returns the file descriptor (a non-negative integer); -1 if
	the file does not exist, if one of the necessary directories
	does not exist or is unreadable, if the file is not readable
	(resp. writable), or if too many files are open.
	*/
	static int open(String pathname, int flags); // throws Exception

	/* Open a file using a FileDescriptor.  The open and create
	methods build a file descriptor and then invoke this method
	to complete the open process.
	*/
	static int open(FileDescriptor *fileDescriptor);

	/* Read bytes from a file. Simulates the unix system call:
	*   int read(int fd, void *buf, size_t count);
	returns the number of bytes actually read; or -1 if an error occurs.
	*/
	static int read(int fd, byte buf[], int count); // throws Exception

	/* Reads a directory entry from a file descriptor for an open directory.
	 Simulates the unix system call:
	 *   int readdir(unsigned int fd, struct dirent *dirp);
	 Note that count is ignored in the unix call.
	 fd the file descriptor for the directory being read;
	 dirp the directory entry into which data should be copied;
	 returns number of bytes read; 0 if end of directory; -1 if the file
	 descriptor is invalid, or if the file is not opened for read access.
	*/
	static int readdir(int fd, DirectoryEntry *dirp); // throws Exception

	/* Obtain information for an open file. Simulates the unix system call:
	*   int fstat(int filedes, struct stat *buf);
	*/
	static int fstat(int fd, Stat *buf); // throws Exception

	/* Obtain information about a named file. Simulates the unix system call:
	*   int stat(const char *name, struct stat *buf);
	*/
	static int stat(String name, Stat *buf); // throws Exception

	/* First commits inodes to buffers, and then buffers to disk.
	Simulates unix system call:
	*   int sync(void);
	*/

	static void sync();

	/* Write bytes to a file. Simulates the unix system call:
	*   int write(int fd, const void *buf, size_t count);
	*/

	static int write(int fd, byte buf[], int count); // throws Exception

	/* Writes a directory entry from a file descriptor for an
	 open directory. Simulates the unix system call:
	 *   int readdir(unsigned int fd, struct dirent *dirp);
	Note that count is ignored in the unix call;
	fd the file descriptor for the directory being read;
	dirp the directory entry into which data should be copied;
	returns number of bytes read; 0 if end of directory; -1 if the file
	descriptor is invalid, or if the file is not opened for read access.
	*/

	static int writedir(int fd, DirectoryEntry *dirp); // throws Exception

	static int link(String oldpath, String newPath);
	static int unlink(const char *pathname);

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

	/* This is an internal variable for the simulator which always
	points to the current ProcessContext.  If multiple processes are
	implemented, then this variable will "point" to different
	processes at different times.
	*/
	static ProcessContext *process;

	static int processCount; // The number of processes.
	static int MAX_OPEN_FILES;

	// should dynamically allocate this array in initialize using
	// MAX_OPEN_FILES, but ok ...
	#define MaxOpenFiles 50
	static FileDescriptor *openFiles[MaxOpenFiles];

	// ??? should be private?
	static int MAX_OPEN_FILE_SYSTEMS;

	// ??? should be private?
	static short ROOT_FILE_SYSTEM;



public:


	// should dynamically allocate this array in initialize using
	// MAX_OPEN_FILE_SYSTEMS, but ok ...
	#define MaxOpenFileSystems 50
	static FileSystem *openFileSystems [MaxOpenFileSystems];

	/* Initialize the file simulator kernel.  This should be the
	 first call in any simulation program.  You can think of this
	 as the method which "boots" the kernel.
	 This method opens the "filesys.conf" file (if provided)
	 and reads any properties
	 given in that file, including the filesystem.root.filename and
	 filesystem.root.mode ("r", "rw").
	*/

	static void initialize();

	static const int EXIT_F=1; // Failure exit status.
	static const int EXIT_S=0; // Success exit status.

	/* End the simulation and exit.
	 Terminates any remaining "processes", flushes all file system blocks
	 to "disk", and exit the simulation program.  This method is generally
	 called by exit() when the last process terminates.  However,
	 it may also be called directly to gracefully end the simlation.
	*/
	static void finalize(int status); // throws Exception

/* Some internal methods. */

private:
  /* Check to see if the integer given is a valid file descriptor
     index for the current process.  Sets errno to EBADF if invalid.
     This is a convenience method for the simulator kernel;
     it should not be called by user programs.
     fd the file descriptor index;
     returns 0 if the file descriptor index is valid; -1 if the file
     descriptor index is not valid
  */
static int check_fd(int fd);

/* Check to see if the integer given is a valid file descriptor
   index for the current process, and if so, whether the file is
   open for reading.  Sets errno to EBADF if invalid or not open
   for reading.
   This is a convenience method for the simulator kernel;
   it should not be called by user programs.
   fd is the file descriptor index;
   returns 0 if the file descriptor index is valid; -1 if the file
   descriptor index is not valid or is not open for reading.
*/
static int check_fd_for_read(int fd);

/* Check to see if the integer given is a valid file descriptor
   index for the current process, and if so, whether the file is
   open for writing.  Sets errno to EBADF if invalid or not open
   for writing.
   This is a convenience method for the simulator kernel;
   it should not be called by user programs.
   fd is the file descriptor index;
   returns 0 if the file descriptor index is valid; -1 if the file
   descriptor index is not valid or is not open for writing.
*/
static int check_fd_for_write(int fd);

/* Get the full path for a file by adding
   the working directory for the current process
   to the beginning of the given path name
   if necessary.
   pathname IS the given path name;
   returns the resulting fully qualified path name
*/
static String getFullPath(String pathname);
static String getDeepestDir(String pathname);

static IndexNode *rootIndexNode;
static IndexNode *getRootIndexNode();

static short findNextIndexNode
  (FileSystem *fileSystem, IndexNode *indexNode, String name,
   IndexNode *nextIndexNode); // throws Exception

static short findIndexNode(String path, IndexNode *inode);

};

