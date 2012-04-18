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
#	-d: If the script fails to gather information
#		about the current filesys.dat, ie because
#		it doesn't exist, this setting will cause
#		the script to use default values.
#		If -s or -t is used, thier respective values
#		will take precedene over the defaults.
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
	skip_proc_check = False

	try:
		##Fix for problem on class VM
		try:
			fsys_stat = os.stat("filesys.dat")
		except:
			raise IOError

		infoproc = subprocess.Popen(["./df"], stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
	except IOError:
		skip_proc_check = True		
	except:
		(e_type, e_value, e_trace) = sys.exc_info()
		print("Message: " + str(e_value))
		print("Check that the user space utilities (especially df) are compiled")
		exit(-1)

	if not skip_proc_check:
		infoproc.wait()
		if infoproc.returncode < 0:
			print("Process df returned with bad stats: " + str(infoproc.returncode))

		info = infoproc.communicate()[0]
	else:
		info = ""

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

		if not ('-noprompt' in sys.argv):
			raw_input("Press <enter> to continue") 

	##Configurable options
	prompt = True
	default = False
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
		elif op[:2] == "-d":
			default = True
		else:
			print("Invalid option %s. Continuing..." % (op))

	if default:
		if int(size) <= 0:
			print("Using default block size: 256")
			size = "256"

		if int(total) <= 0:
			print("Using default total blocks: 30")
			total = "30"
		
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
	
