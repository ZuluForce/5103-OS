/* RandomAccessFile - provides abstraction of byte-oriented files with
   basic I/O operations. Implemented on local Unix FS. */

#include <io_types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <RandomAccessFile.h>
#include <SuperBlock.h>

void RandomAccessFile::writeShort(short s ) {
  write (fd, &s, sizeof(short));
}

void RandomAccessFile::writeInt(int i) {
  write (fd, &i, sizeof(int));
}

// Kludge alert ... seems to work - adapted from Java
void RandomAccessFile::Write( byte* b) {
  int len;
  int val=0;
  if (b==0) {
    len=1; 
    write (fd, &val, 1);
  }
  else {
    len = SuperBlock::getBlockSize() ;
    write (fd, b, len);
  }
}

// Not exactly a mirror of Java semantics but close enough
// Always seems to be reading BlockSize, so it is probably ok.
void RandomAccessFile::readFully( byte* b) {
  read (fd, b, SuperBlock::getBlockSize());
}

int RandomAccessFile::readInt() {
  int i;
  read (fd, &i, 4);
  return i;
}
void RandomAccessFile::skipBytes (int l) {
  lseek (fd, l, SEEK_SET);
}

void RandomAccessFile::Seek (int l) {
  lseek (fd, l, SEEK_SET);
}

void RandomAccessFile::Close () {
  close (fd);
}

short RandomAccessFile::readShort () {
  short i;
  read (fd, &i, sizeof(short));
  return i;
}

RandomAccessFile::RandomAccessFile (String name, String mode) {
  int mo;
  if (!strcmp (mode, "rw"))
    mo = O_RDWR;
  else if (!strcmp (mode, "r"))
    mo = O_RDONLY;
  else if (!strcmp (mode, "w"))
    mo = O_WRONLY;
  fd = open (name, mo|O_CREAT, 0600);
}

