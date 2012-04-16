output = "./tests/should_error/badfname.result"

description = "Test that invalid filenames are handled appropriately"

options = "rebuild"

execlist = ['echo "Hello.txt" | ./tee hello',\
			'echo "Hello" | ./tee /hello.txt',\
			'./ls','./ls /directory','./ls //',\
			'./rm nofile','./rm /nofile',\
			'./cat hello','./cat /hello.txt',\
			'./ln hello /link','./ln /hello.txt badname -s',\
			'./rm /hello.txt']

description=\
"""Most will give an error. Two of the ls calls will work. The creation of
the symolic link will work but not the hard link."""
