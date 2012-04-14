/* DirectoryEntry: abstraction of a directory entry for the simulated file system */

#include <DirectoryEntry.h>
#include <stdio.h>

DirectoryEntry::DirectoryEntry() {
	d_ino = 0;
	d_name = new byte[MAX_FILENAME_LENGTH + 1];
	d_name_len = MAX_FILENAME_LENGTH;

	for (int i=0; i<MAX_FILENAME_LENGTH+1; i++)
		d_name[i] = '\0';
}

DirectoryEntry::DirectoryEntry(short ino, String name) {
	d_ino = 0;
	d_name = new byte[MAX_FILENAME_LENGTH + 1];
	d_name_len = MAX_FILENAME_LENGTH + 1;

	for (int i=0; i<MAX_FILENAME_LENGTH+1; i++)
		d_name[i] = '\0';

	setIno(ino);
	setName(name);
}

void DirectoryEntry::setIno(short newIno) {
  d_ino = newIno;
}

short DirectoryEntry::getIno() {
  return d_ino;
}

void DirectoryEntry::setName(String newName) {
  for(int i = 0; i < MAX_FILENAME_LENGTH && i < strlen(newName); i ++)
    if(i < strlen(newName))
      d_name[i] = (byte)newName[i];
    else
      d_name[i] = (byte)0;
  d_name_len = MAX_FILENAME_LENGTH; //strlen(newName)
  d_name[d_name_len] = (byte)0;
}

String DirectoryEntry::getName() {
  StringBuffer *s = new StringBuffer(MAX_FILENAME_LENGTH);
  for(int i = 0; i < MAX_FILENAME_LENGTH; i ++) {
      if (d_name[i] == (byte)0)
        break;
      s->append((char)d_name[i]);
    }
  return s->toString();
}

void DirectoryEntry::write(byte *buffer , int offset) {
	//    buffer[offset] = (byte)(d_ino >>> 8);
	buffer[offset] = (byte)(d_ino >> 8);
	buffer[offset+1] = (byte) d_ino;
	for(int i = 0; i < MAX_FILENAME_LENGTH; i ++)
		buffer[offset+2+i] = d_name[i];
}

void DirectoryEntry::read(byte *buffer , int offset) {
  int hi = buffer[offset] & 0xff;
  int lo = buffer[offset+1] & 0xff;
  d_ino = (short)(hi << 8 | lo);
  for(int i = 0; i < MAX_FILENAME_LENGTH; i ++)
    d_name[i] = buffer[offset+2+i];
  d_name[MAX_FILENAME_LENGTH] = (byte)0;
}

String DirectoryEntry::toString() {
  StringBuffer *s = new StringBuffer("DirectoryEntry[");
  s->append(getIno());
  s->append(',');
  s->append(getName());
  s->append(']');
  return s->toString();
}

bool DirectoryEntry::checkValid(String name) {
	if ( strlen(name) > DirectoryEntry::MAX_FILENAME_LENGTH )
		return false;

	return true;
}

