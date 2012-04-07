/* DirectoryEntry: abstraction of a directory entry for the simulated file system
 */

#ifndef DIR_H
#define DIR_H

#include <io_types.h>

class DirectoryEntry {
public:
	static const int MAX_FILENAME_LENGTH = 14;

	// size of a directory entry (on disk) in bytes.
	static const int DIRECTORY_ENTRY_SIZE = MAX_FILENAME_LENGTH + 2;

	short d_ino ;    // i-node number for this DirectoryEntry

	byte *d_name; // file name for this DirectoryEntry
	int d_name_len; // file name length ...

	DirectoryEntry();

	// Constructs a DirectoryEntry for the given inode and name.
	DirectoryEntry(short ino, String name);

	// Sets the inode number for this DirectoryEntry
	void setIno(short newIno);

	// Gets the inode number for this DirectoryEntry
	short getIno();

	// Sets the name for this DirectoryEntry
	void setName(String newName);

	// Gets the name for this DirectoryEntry
	String getName();

	/* Writes a DirectoryEntry to the specified byte array at the specified
	* offset; buffer is the byte array to which the directory entry should be written;
	* offset is the offset from the beginning of the buffer to which the
	* directory entry should be written
	*/
	void write(byte *buffer, int offset);

	/* Reads a DirectoryEntry from the spcified byte array at the specified
	* offse; buffer is the byte array from which the directory entry should be read;
	* offset is the offset from the beginning of the buffer from which the
	* directory entry should be read
	*/
	void read(byte *buffer, int offset);

	// Converts a DirectoryEntry to a printable string.
	String toString();
};
#endif
