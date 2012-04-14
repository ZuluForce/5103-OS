#!/usr/bin/python

################################################
# Tired of removing and rebuilding filesys.dat
# Well here you go...
#
# This will query for information on the
# current filesys.dat using the df program
# and will rebuild it with the same parameters
#
#Command Line Options:
#	-noprompt: Do not prompt the user before
#			rebuilding
#
#	-s#: Use the number after -s as the new
#			block size.
#
#	-t#: Use # as the new total block count
#
# If filesys.dat does not exist when ./df is run
# it will print an error but you can continue and
# build a new filesys.dat if you have specified
# block size and total count values
#################################################

import os,re,sys,subprocess
import traceback

class dummyMatch:
	def __init__(self):
		self.value = "0"

	def group(self, index = 0):
		return self.value

if __name__ == '__main__':
	try:
		infoproc = subprocess.Popen(["./df"], stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
	except:
		(e_type, e_value, e_trace) = sys.exc_info()
		print("Exception: " + str(e_type))
		print("Message: " + str(e_value))
		exit(-1)

	infoproc.wait()
	if infoproc.returncode < 0:
		print("Process df returned with bad stats: " + str(infoproc.returncode))

	info = infoproc.communicate()[0]

	blkSize = re.compile("Block Size: (\d+)")
	totalBlocks = re.compile("Total Blocks: (\d+)")

	bsize_match = blkSize.search(info)
	blocks_match = totalBlocks.search(info)

	if ( bsize_match == None or blocks_match == None):
		print("Failed to gather information about current filesystem")
		print("Stdout of executed process:\n" + info)
		print("Either something is really wrong or there currently is no filesys.dat")
		print("You can continue if you have specified -s and -t values")
		bsize_match = dummyMatch()
		blocks_match = dummyMatch()
		raw_input("Press <enter> to continue") 

	##Configurable options
	prompt = True
	size = bsize_match.group(1)
	total = blocks_match.group(1)

	##Parse command line options
	for op in sys.argv[1:]:
		if op == "-noprompt":
			prompt = False
			continue

		if len(op) < 2:
			print("Invalid option %s. Continuing..." % (op))
			continue

		if op[:2] == "-s":
			size = op[2:]
		elif op[:2] == "-t":
			total = op[2:]
		else:
			print("Invalid option %s. Continuing..." % (op))

	if ( int(size) <= 0 or int(total) <= 0):
		print("Invalid values for block size or block total")
		print("Block Size: %s  Block Total: %s" % (size, total))
		exit(-1)

	print("Recreating filesys.dat with these parameters:")
	print("\tBlock Size: " + size)
	print("\tTotal Blocks: " + total)

	if prompt:
		raw_input("Press <enter> to continue\n")

	size = int(size)
	total = int(total)

	print("rm -f filesys.dat")
	os.system("rm -f filesys.dat")
	exec_str = "./mkfs filesys.dat %d %d" % (size, total)

	print(exec_str)
	os.system(exec_str)
	
