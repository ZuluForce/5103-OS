/* RandomAccessFile - provides abstraction of byte-oriented files with
   basic I/O operations. Implemented on local Unix FS. */

#ifndef RA_H
#define RA_H

#include <io_types.h>
#include <stdio.h>

class RandomAccessFile {
  int fd;
 public:
  void writeShort( short);
  void writeInt( int) ;
  void Write( byte*);
  void Seek (int);
  int readInt();
  void skipBytes (int);
  short readShort ();
  void readFully (byte*);  // see Java spec for meaning
  RandomAccessFile (String, String);
  void Close();
};

#endif
