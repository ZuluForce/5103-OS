output = "./tests/boundary_tests/max_fs.result"

description = "Test that all the regular operations work with a filesize that has all its direct inode blocks allocated and filled"

options="estop"

execlist = ['./rebuild.py -noprompt -s128 -t30',\
			'./df',\
			'cat ./tests/moby.txt | ./tee /moby_dick.txt',\
			'./df',\
			'./mkdir /Planeman',\
			'./mkdir /Planeman/books',\
			'./mkdir /tmp',
			'./df','./ls /',\
			'./ln /moby_dick.txt /Planeman/books/moby.txt',\
			'./ln /moby_dick.txt /tmp/book.txt -s',\
			'./df','./ls /Planeman/books',\
			'./rm /moby_dick.txt',\
			'./df','./cat /Planeman/books/moby.txt',\
			'./rm /tmp/book.txt',\
			'echo "I get rid of your literature..." | ./tee /Planeman/books/moby.txt',\
			'./df','./cat /Planeman/books/moby.txt',\
			'./rm /Planeman/books/moby.txt','./df']
			
expected =\
"""Everything should work"""
