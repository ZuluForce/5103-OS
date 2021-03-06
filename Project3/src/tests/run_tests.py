#!/usr/bin/python
import os,sys,re
import subprocess
import traceback

class testMod:
	def __init__(self, num):
		try:
			self.ref = eval("t" + str(num))
		except:
			print("Failed to get module reference. testMod constructor")
			exit(-1)

		if not ('ignore_error' in dir(self.ref)):
			self.ref.ignore_error = []

		if not ('expected' in dir(self.ref)):
			self.ref.expected = ""

		if not ('options' in dir(self.ref)):
			self.ref.options = ""

		if not ('execlist' in dir(self.ref)):
			print("Missing execlist in test module: t%d" % (num))
			print("This is a required attribute")
			exit(-1)

		if not ('output' in dir(self.ref)):
			print("Missing output in test module: t%d" % (num))
			print("This is a required attribute")
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

	def getDescription(self):
		try:
			return self.ref.description
		except:
			return None

	def inIgnoreList(self, str):
		return str in self.ref.ignore_error

class execHandler:
	def __init__(self, cmd):
		self.depends = None
		self.cmd = cmd

		##Check if there is a redirection pipe
		p = cmd.find("|")
		if ( p >= 0 ):
			self.cmd = cmd.split('|',1)

			##This command depends on this ones output
			self.depends = execHandler(self.cmd[1])
			self.cmd = self.cmd[0]
			
		##Separate the executable from the argument list
		self.cmd = self.cmd.strip()
		self.cmd = self.cmd.split(' ')

	def printCmd(self):
		print("Command: " + str(self.cmd))

	def run(self, input=None):
		#self.printCmd()
		s_out = ""
		s_err = ""
		try:
			proc = subprocess.Popen(self.cmd,\
					stdin=subprocess.PIPE,\
					stdout=subprocess.PIPE,\
					stderr=subprocess.PIPE)
			if input != None:
				(s_out,s_err) = proc.communicate(input)
			else:
				(s_out,s_err) = proc.communicate()

			proc.wait()
			##(s_out,s_err) = proc.communicate()
		except Exception as e:
			e_type, e_value, e_trace = sys.exc_info()
			print("Exception: " + str(e))
			print("\tType: " + str(e_type))
			print("\tLine #: " + str(traceback.tb_lineno(e_trace)))
			return (s_out,s_err,-1)
	
		if proc.returncode != 0:
			print("Error executing the test: " + ' '.join(self.cmd))
			print("Return Value: " + str(proc.returncode))
			print("If the script doesn't stop this was likely an intentional error")
			return (s_out,s_err, proc.returncode)

		status = 0
		if ( self.depends ):
			(s_out, s_err, status) = self.depends.run(s_out)

		return (s_out, s_err,status)

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

	opClean_re	 = re.compile("\\bclean\\b")
	opStop_re	 = re.compile("\\bestop\\b")
	opRebuild_re = re.compile("\\brebuild\\b")
	opSaveFS_re	 = re.compile("\\bsavefs\\b")


	for testNum in range(numTests):
		opClean		= False
		opStop		= False
		opRebuild	= False
		opSaveFS	= False

		Tobj = testMod(testNum)

		output	 = Tobj.getOutput()
		options	 = Tobj.getOptions()
		execlist = Tobj.getExecList()
		expected = Tobj.getExpected()
		description = Tobj.getDescription()

		print("\nTest output: " + output)
		print("Test Options: " + options)
		print("Exec List: ")
		for e in execlist:
			print("\t" + e)

		print("\nExpected output: " + expected)

		##Open Output file before anything else
		try:
			outfile = open(output, "w+")
		except IOError as (errno,strerror):
			print("Failed to open output file: " + strerror)
			raw_input("Press <enter> to continue with other tests")
			continue

		if description:
			outfile.write("Test Description:\n\t")
			outfile.write(description + "\n\n")

		match = opClean_re.search(options)
		if match:
			opClean = True

		match = opStop_re.search(options)
		if match:
			opStop = True

		match = opRebuild_re.search(options)
		if match:
			opRebuild = True

		match = opSaveFS_re.search(options)
		if match:
			opSaveFS = True

		##Log the options to the screen
		print("Parsed Options Settings:")
		print("\tClean: " + str(opClean))
		print("\tStop on Error: " + str(opStop))
		print("\tRebuild FS: " + str(opRebuild))		

		if ( opRebuild ):
			print("\n##---- Rebuilding the filesystem ---- ##")
			os.system("./rebuild.py -noprompt -d")
			print("")

		proc = None

		counter = 0
		outputs = []
		errors = []
		return_codes = []

		for stmnt in execlist:
			exec_msg = "[::Harness --> Output ID: %d]: %s" % (counter,stmnt)
			print(exec_msg)
			outfile.write(exec_msg + "\n")

			proc = execHandler(stmnt)
			(s_out, s_err,status) = proc.run()

			if status != 0:
				if not Tobj.inIgnoreList(stmnt):
					print("Error executing command\n")
					if opStop:
						raw_input("Press <enter> to continue")

			outputs.append(s_out)
			errors.append(s_err)
			return_codes.append(status)

			counter += 1

		print("Writing output/errors to logfile: " + output)
		outfile.write("\n**Notice: Some programs will default output to stderr to avoid bufferring\n")
		for i in range(len(execlist)):
			out_msg = "[Output ID: %d]:\n%s\n" % (i,outputs[i])
			status_msg = "[Return Status: %d]\n" % ( return_codes[i])
			err_msg = "[Error Output]:\n%s\n\n" % (errors[i])
			
			outfile.write("/* ------------------------------------ */\n")
			outfile.write(out_msg)
			outfile.write(status_msg)
			outfile.write(err_msg)

		outfile.write("[Expected Output]:\n\t" + expected + "\n")	

		if opSaveFS:
			outfile.write("Saving filesystem as filesys_save%d.dat" % (testNum))
			os.system("cp filesys.dat filesys_save%d.dat" % (testNum))
		
		outfile.close()

		##This terminates the given test
