output = "./tests/unlink_tests/basic.result"

description = "Test the basic functionality of unlink and the userspace rm"

options = "rebuild estop"

ignore_error = ['./cat /link2.txt']

execlist = ['echo "Hello" | ./tee /hello.txt',\
			'cat ./tests/64_bytes.txt | ./tee /64file.txt',\
			'./mkdir /newdir',\
			'cat ./tests/64_bytes.txt | ./tee /newdir/64file.txt',\
			'./ln /hello.txt /link1.txt','./ln /newdir/64file.txt /link2.txt -s',\
			'./ls /', './ls /newdir',\
			'./cat /hello.txt',\
			'./rm /hello.txt','./ls /',\
			'./cat /link1.txt',\
			'./rm /link1.txt','./ls /',\
			'./cat /newdir/64file.txt',\
			'./rm /newdir/64file.txt',\
			'./cat /link2.txt','./rm /link2.txt','./rm /64file.txt',\
			'./ls /','./ls /newdir',\
			'./df']
			
expected =\
"""This creates two files, one in the main directory and another in a sub-directory.
A hard link is created to the file in the root directory. A symbolic link is made to
the file in the sub-directory.

After the original root file is removed the hard link should still cat out the file.
After the original sub-directory file is removed the symbolic link should give a bad
link error.

In the end the block allocation should show 3 inode and 3 data blocks allocated."""
