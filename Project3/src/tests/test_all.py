#!/usr/bin/python
import os,sys,re

if __name__ == '__main__':
	if ( len(sys.argv) < 3 ):
		print("Usage: %s <testdir> <execution_dir>" % (sys.argv[0]))
		exit(1)

	##The testdir should be relative to the execution directory
	##because tests will be loaded after switching to the execution
	##directory.

	os.chdir(sys.argv[2])
	cwd = os.getcwd()
	print("Switched to executing directory: " + cwd)

	full_tests_path = os.path.join(cwd, sys.argv[1])

	test_list = os.listdir(sys.argv[1])
	if (test_list == None):
		print("Failed to get listing of test directory")
		exit(-1)

	test_re = re.compile("^.+?\.test.py")

	print("Searching for tests in " + full_tests_path)
	for file in test_list:
		full_path = os.path.join(full_tests_path, file)

		if os.path.isdir(full_path):
			print("Skipping(%s): not a file" % (file))
			continue

		match = None
		match = test_re.match(file)
		if match == None:
			print("Skipping(%s): not a test file" % (file))
			continue

		print("Importing test: " + file)
		##Chop off the .py for importing
		import_file = full_path[:-3]
		import_stmt = "import " + import_file + " as test"
		try:
			exec(import_stmt)
		except:
			print("Failed to import: " + import_file)
			print("Continuing...")
			continue
		
		##Get necessary information
		try:
			output = test.output
			execStrs = test.execlist
		except:
			print("Missing necessary information from test: " + file)
			continue
		
		options = test.options
		expected = test.expected

		if (type(output) != str):
			print("output should be a string")
		if (type(options) != str):
			print("options should be a string")
		if (type(expected) != str):
			print("expected should be a string")
		if (type(execlist) != list or type(execlist) != tuple):
			print("execlist should be a list or tuple")

		print ("||*---- Test Paramaters ----*||")
		print ("||Output: %s ||" % (output))
		print ("||Options: %s ||" % (options))
		print ("||*-------------------------*||")
		
