output = "./tests/unlink_tests/dealloc2.result"

description =\
"""Checking for proper deallocation of datablocks when a directory's file size goes
below a block boundary."""

options = "estop"

execlist = ['./rebuild.py -noprompt -s64 -t20',\
			'echo "Hello" | ./tee /hello1.txt',\
			'echo "Hello" | ./tee /hello2.txt',\
			'./ls /','./df',\
			'./ln /hello2.txt /link.lnk -s',\
			'./ls /','./df',\
			'./rm /hello1.txt','./ls /','./df',\
			'./rebuild.py -noprompt -s256 -t20']
			
expected=\
"""After hello1 and hello2 are added to the directory it will have one completely
full datablock. When the symbolic link file is added it will occupy a new data block.
This will result in the datablock allocation going up by 2 (1 for symlink, 1 for new
directory block) and 1 new inode for the symlink.

After hello1.txt is removed it should go back down by 2 datablocks and 1 inode."""
