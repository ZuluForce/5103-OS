#!/usr/bin/python

import os,re,sys

#########################################################################
# How to use this testing harness
# Use: run_all.py <bindir> <test_configs_dir> <optional: output_dir>
#
# The script will first try to chdir to <bindir>
#
# This script will treat each subdirectory of <test_configs_dir> as
# a single test. It will try and execute a makefile contained within
# each of these directories. In order for this to work the makefile
# must be of the form (m|M)ake*.
#
# After executing all tests, if the optional output_dir was named the
# script tries to invoke makeGraphs.py on each of these output directories.
# This script uses a fixed path for makeGraphs.py. It should be in the
# Project2/scripts/ directory while this script should be executing in
# the bin directory.
#
# If execution of a test fails the script will wait for user input to
# continue. This is necessary because when running many tests it would
# be difficult to read through terminal output for test status.
############################################################################

if ( len(sys.argv) < 3 ):
	print("Usage: %s <bindir> <test_configs_dirname> <optional: output_dir>" % (sys.argv[0]))
	exit(-1)

if (not os.chdir(sys.argv[1])):
	print("Failed to switch to directory: " + sys.argv[1])

cwd = os.getcwd()
print("Switched to directory: " + cwd)

test_dirs = os.listdir(sys.argv[2])
if (test_dirs == None):
	print("Failed to get listing for test_configs folder")
	exit(-1)

if ( len(sys.argv) > 3 ):
	output_dirs = os.listdir(sys.argv[3])
else:
	output_dirs = None

make_re = re.compile("^(?:M|m)ake.*$")

full_configdir_path = os.path.join(cwd, sys.argv[2])

if ( output_dirs != None ):
	full_outdir_path = os.path.join(cwd, sys.argv[3])

for dir in test_dirs:
	if not os.path.isdir(os.path.join(full_configdir_path, dir)):
		print("Skipping (%s): not a directory" % (dir))
		continue

	test_path = os.path.join(sys.argv[2],dir)
	test_content = os.listdir(test_path)

	makefile = None

	print("Test dir contents: " + str(test_content))
	for file in test_content:
		print("Trying to match: " + str(file))
		m = make_re.match(file)

		if (m != None):
			makefile = m.group(0)
			break

	if (makefile == None):
		print("Couldn't find makefile in test directory: %s" % (test_path))
		continue

	##Otherwise execute the makefile
	exec_str = "make -f %s" % (os.path.join(test_path, makefile))
	print("\nExecuting: " + exec_str)
	error = os.system(exec_str)
	
	if ( error > 0 ):
		print("Error Executing: " + exec_str)
		print("Return Code: " + str(error))
		raw_input("Press <enter> to continue")

if output_dirs == None:
	exit(0)

try:
	import numpy
	import matplotlib
except:
	print("Required libraries: numpy,matplotlib are not installed")
	print("Skipping graph creation")
	print("Check the readme on instructions for fulfilling dependencies\n")
	exit(0)

for dir in output_dirs:
	if not os.path.isdir(os.path.join(full_outdir_path,dir)):
		print("Skipping (%s): not a directory" % (dir))
		continue
		
	out_path = os.path.join(sys.argv[3], dir)
	out_contents = os.listdir(out_path)
	
	graphMaker = "../scripts/makeGraphs.py"
	print("\nExecuting: " + "python %s %s" % (graphMaker, out_path))
	os.system("python %s %s" % (graphMaker,out_path))
	
	
