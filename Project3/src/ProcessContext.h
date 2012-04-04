/* A process context.  This contains all information needed
   by the file system which is specific to a process.
*/

#ifndef PC_H
#define PC_H

#include <FileDescriptor.h>
#define MaxOpenFiles 50

class ProcessContext
{
 public:
  int errno; // error number of last error

 private:
  short uid ; // process uid
  short gid;  // process gid
  String dir; // the working directory for the process
  short umask ; // the umask for the process.

 public:
  static int MAX_OPEN_FILES; // the maximum number of files a process may have open
  FileDescriptor *openFiles[MaxOpenFiles]; // open file array
  int num_files;
  ProcessContext();
  ProcessContext(short newUid, short newGid, String newDir, 
		 short newUmask);

  void setUid(short newUid);
  short getUid();
  void setGid( short newGid );
  short getGid();
  void setDir( String newDir ); // set process working dir
  String getDir(); // get process working dir
  void setUmask(short newUmask);
  short getUmask();
};

#endif
