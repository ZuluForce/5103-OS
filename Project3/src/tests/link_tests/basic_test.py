##The output file is relative to the execution
##directory since the python script does not
##track where it loaded tests from.
output = "tests/link_tests/basic_test.result"

## I used """ to make it multiline but this is not required
description = """This is a test of a simple linking operation.
                All it does is create a 64 byte file and hard links
                it from the root directory and then from a sub
                directory. After this it cats the original and all
                the other links to show that they have the same
                content."""

##Option Values:
##	clean: I forgot what this was supposed to do
##	estop: Halts execution on execution error
##	rebuild: Will try to execute the rebuild script. If EStop
##			is enabled, failing to do this will halt
##
##      savefs: Before going to another test it will copy the current
##              filesys.dat so it is saved for tinkering later.

options = "clean estop savefs"
execlist = ['./mkfs filesys.dat 256 20',\
		'./mkdir /school/',\
		'cat ./tests/64_bytes.txt | ./tee /64_byte_file.txt',\
		'./ln /64_byte_file.txt /link64.txt',\
		'./ln /64_byte_file.txt /school/link_64.txt',\
		'./cat /64_byte_file.txt',\
		'./cat /link64.txt',\
		'./cat /school/link_64.txt',]

expected = """/* ---- No expected output for this test ---- */"""
