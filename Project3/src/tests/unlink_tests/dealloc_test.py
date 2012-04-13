output = "./tests/unlink_tests/dealloc.result"

description = "Testing proper data and inode block deallocation using unlink and rm"

options = "rebuild estop"

ignore_error = []

execlist = ['./df',\
			'echo "Hello" | ./tee /hello.txt',\
			'./ls /','./df',\
			'./rm /hello.txt','./df',\
			'echo "Hello" | ./tee /hello.txt',\
			'./ln /hello.txt /slink1.txt -s',\
			'./ln /hello.txt /hlink1.txt',\
			'./ls','./df',\
			'./rm /hello.txt','./ls /','./df',\
			'./rm /slink1.txt','./ls /','./df',\
			'./rm /hlink1.txt','./ls /','./df']
			
			
expected =\
"""In the end there should only be 1 data and inode block allocated."""
