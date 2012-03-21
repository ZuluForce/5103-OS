#!/usr/bin/python

import os,re,sys

if ( len(sys.argv) < 3 ):
	print("Usage: %s <bindir> <test_configs_dirname>" % (sys.argv[0]))
	exit(-1)

if (not os.chdir(sys.argv[1])):
	print("Failed to switch to directory: " + sys.argv[1])

cwd = os.getcwd()
print("Switched to directory: " + cwd)

test_dirs = os.listdir(sys.argv[2])
if (test_dirs == None):
	print("Failed to get listing for test_configs folder")
	exit(-1)

make_re = re.compile("^(?:M|m)ake.*$")

full_configdir_path = os.path.join(cwd, sys.argv[2])

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
	print("Executing: " + "make -f " + os.path.join(test_path,makefile))
	os.system("make -f %s" % (os.path.join(test_path, makefile)))
