/* A process context.  This contains all information needed
   by the file system which is specific to a process.
*/

#include <ProcessContext.h>
#include <FileDescriptor.h>

int ProcessContext::MAX_OPEN_FILES;

// Construct a process context.  By default, uid=1, gid=1, dir="/root",
// and umask=0000.
ProcessContext::ProcessContext()
{
  errno = 0;
  uid = 1;
  gid = 1;
  dir = "/root";
  umask = 0000; 

  for (int i=0; i<MaxOpenFiles; i++)  { 
    openFiles[i] = new FileDescriptor;
    openFiles[i] = null;
  }
}

// Construct a process context and specify uid, gid, dir, and umask.
ProcessContext::ProcessContext(short newUid , short newGid , String newDir , 
				short newUmask)
{
  errno = 0;
  uid = newUid;
  gid = newGid;
  dir = newDir;
  umask = newUmask;
  for (int i=0; i<MaxOpenFiles; i++) {
    openFiles[i] = new FileDescriptor;
    openFiles[i] = null;
  }
}

void ProcessContext::setUid(short newUid)
{
  uid = newUid;
}

short ProcessContext::getUid()
{
  return uid;
}

void ProcessContext::setGid(short newGid)
{
  gid = newGid;
}

short ProcessContext::getGid()
{
  return gid;
}

void ProcessContext::setDir(String newDir)
{
  dir = newDir;
}

String ProcessContext::getDir()
{
  return dir;
}

void ProcessContext::setUmask(short newUmask)
{
  umask = newUmask;
}

short ProcessContext::getUmask()
{
  return umask;
}
