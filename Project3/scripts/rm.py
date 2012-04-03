#!/usr/bin/python

import os,sys

if ( len(sys.argv) < 3 ):
	print("Usage: %s <directory> <rm pattern>" % (sys.argv[0]))
	exit(-1)
	
os.chdir(sys.argv[1])

cwd = os.getcwd()
dir_list = os.listdir(os.getcwd())

for f in dir_list:
	if (not os.path.isdir(os.path.join(cwd,f)) ):
		print("Skipping (%s): Not a directory" % (f))
		continue

        print("Executing: rm -f %s/%s" % (f,sys.argv[2]))
	os.system("rm -f %s/%s" % (f,sys.argv[2]))
