#!/usr/bin/python
import os,sys,re

class testMod:
	def __init__(self, num):
		try:
			self.ref = eval("t" + str(num))
		except:
			print("Failed to get module reference. testMod constructor")
			exit(-1)

	def getOutput(self):
		return self.ref.output

	def getOptions(self):
		try:
			return self.ref.options
		except:
			return None

	def getExecList(self):
		return self.ref.execlist

	def getExpected(self):
		try:
			return self.ref.expected
		except:
			return None

if __name__ == '__main__':
	if ( len(sys.argv) < 3 ):
		print("Usage: %s <exec_dir> <test_dir/s>" % (sys.argv[0]))
		exit(1)
	##The testdir/s should be relative to the scripts directory

	test_re = re.compile("^(.+?_test).py$")

	numTests = 0
	for test_dir in range(2, len(sys.argv)):
		sys.path.insert(0,sys.argv[test_dir])

		tests = os.listdir(sys.argv[test_dir])
		if (tests == None):
			print("Failed to get listing of test directory: " +\
					sys.argv[test_dir])

		for test in tests:
			match = test_re.match(test)
			if (match == None):
				continue

			test = match.group(1)

			importStr = "import %s as t%d" % (test,numTests)
			try:
				exec(importStr)
			except ImportError:
				print("Failed to import test: %s from directory %s"\
					% (test,sys.argv[test_dir]))
				raw_input("Press <enter> to continue")

			print("Loaded test: " + test)
			numTests += 1

	##Now that all test modules are loaded we can start
	##switch to the executing directory
	os.chdir(sys.argv[1])
	print("Switched to executing directory:\n\t" + os.getcwd())

	opClean_re	 = re.compile("\bclean\b")
	opStop_re	 = re.compile("\bEStop\b")
	opRebuild_re = re.compile("\brebuild\b")

	opClean		= False
	opStop		= False
	opRebuild	= False

	for testNum in range(numTests):
		Tobj = testMod(testNum)

		output	 = Tobj.getOutput()
		options	 = Tobj.getOptions()
		execlist = Tobj.getExecList()
		expected = Tobj.getExpected()

		print("\nTest output: " + output)
		print("Test Options: " + options)
		print("Exec List: ")
		for e in execlist:
			print("\t" + e)

		print("\nExpected output: " + expected)

		##Open Output file before anything else
		try:
			outfile = open(output, "w+")
		except IOError as e:
			print("Failed to open output file: " + e.value)
			raw_input("Press <enter> to continue")
			continue

		match = opClean_re.search(options)
		if match:
			opClean = True

		match = opStop_re.search(options)
		if match:
			opStop = True

		match = opRebuild_re.search(options)
		if match:
			opRebuild = True

		##Log the options to the screen
		print("Parsed Options Settings:")
		print("\tClean: " + str(opClean))
		print("\tStop on Error: " + str(opStop))
		print("\tRebuild FS: " + str(opRebuild))		
