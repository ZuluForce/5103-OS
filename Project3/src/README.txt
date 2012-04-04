To compile:
1)
$ make
This builds the kernel and other relevant object files

2)
$ make all_apps
This builds all the user applications like mkfs, ls and so on.

To run:
Start by creating a filesystem using mkfs:
./mkfs filesys.dat 256 1000

Creates a filesystem in filesys.dat with 256 byte blocks and 1000 blocks in total.

You can now run the various user application and they operate on filesys.dat:
For example:
./ls /

./mkdir /foo

and so on..

