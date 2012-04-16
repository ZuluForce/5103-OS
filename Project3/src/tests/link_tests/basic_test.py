##The output file is relative to the execution
##directory since the python script does not
##track where it loaded tests from.
output = "tests/link_tests/basic_test.result"

## I used """ to make it multiline but this is not required
description =\
"""This is a test of a simple linking operation. All it does is create a 
64 byte file and hard links it from the root directory and then from a sub 
directory. After this it cats the original and allthe other links to show
that they have the same content."""

##Option Values:
##	estop: Halts execution on execution error. Should always use this.
##
##	rebuild: Will try to execute the rebuild script. If EStop
##			is enabled, failing to do this will halt
##
##	savefs: This will copy and save filesys.dat after execution of all
##			the commands. This is usefull if you want to run many tests
##			but still be able to debug the state of each post test
##			filesystem.
##
options = "estop savefs"

## This is a list of commands that you wish to ignore errors for. This should be an
## exact copy of the command used in execlist below. This is useful when you want to
## detect all errors and have the estop option set but you have one particular command
## which you know will error and you want to display that but not stop the execution
## flow.
##
## Example: ignore_error = ['./cat /non_existent_file.txt', './mkdir /']
ignore_error = []


## List of execution statements just as they would be written in the terminal
execlist = ['./mkfs filesys.dat 256 20',\
		'./mkdir /school/',\
		'cat ./tests/64_bytes.txt | ./tee /64_bytes.txt',\
		'./ln /64_bytes.txt /link64.txt',\
		'./ln /64_bytes.txt /school/link_64.txt',\
                './ls /','./ls /school',\
		'./cat /64_bytes.txt',\
		'./cat /link64.txt',\
		'./cat /school/link_64.txt',]

## You can put anything here. It gets printed at the bottom of the result file
expected = """/* ---- No expected output for this test ---- */"""
