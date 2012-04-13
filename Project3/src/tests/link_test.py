##The output file is relative to the execution
##directory since the python script does not
##track where it loaded tests from.
output = "link.test.result"

##Option Values:
##	clean:
##	EStop: Halts execution on error
##	rebuild: Will try to execute the rebuild script. If EStop
##			is enabled, failing to do this will halt
options = "clean EStop"
execlist = ['./mkfs filesys.dat 256 20',\
		'./mkdir /school/',\
		'cat ../tests/64_bytes.txt | ./tee /64_byte_file.txt',\
		'./ln /64_byte_file.txt /school/link_64.txt',\
		'./cat /64_byte_file.txt',\
		'./cat /school/link_64.txt',]

expected = """/* ---- No expected output for this test ---- */"""
