#!/usr/bin/python

#############################################
#Tired of removing and rebuilding filesys.dat
#Well here you go...
#
#This will query for information on the
#current filesys.dat using the df program
#and will rebuild it with the same parameters
#
#Future improvements:
#	If you enter command line parameters for
#	blocksize and block count it will delete
#	the current filesys.dat and rebuild it
#	with your specified parameters.
#############################################

import os,re,sys,subprocess

if __name__ == '__main__':
	infoproc = subprocess.Popen(["./df"], stdout=subprocess.PIPE)

	info = infoproc.communicate()[0]

	blkSize = re.compile("Block Size: (\d+)")
	totalBlocks = re.compile("Total Blocks: (\d+)")

	bsize_match = blkSize.search(info)
	blocks_match = totalBlocks.search(info)

	if ( bsize_match == None or blocks_match == None):
		print("Failed to gather information about current filesystem")
		print("Stdout of executed process:\n" + info)
		exit(-1)

	size = bsize_match.group(1)
	total = blocks_match.group(1)
	print("Recreating filesys.dat with these parameters:")
	print("\tBlock Size: " + size)
	print("\tTotal Blocks: " + total)
	raw_input("Press <enter> to continue\n")

	size = int(size)
	total = int(total)

	print("rm -f filesys.dat")
	os.system("rm -f filesys.dat")
	exec_str = "./mkfs filesys.dat %d %d" % (size, total)

	print(exec_str)
	os.system(exec_str)
	
