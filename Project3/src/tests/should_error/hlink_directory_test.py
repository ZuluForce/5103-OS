output = "./tests/should_error/hlink_dir.result"

description = "Test that hard linking directories results in an error"

options = "rebuild"

execlist = ['./mkdir /home',\
			'./ln /home /home_link.lnk',\
			'./ls /',\
			'echo "Hello" | ./tee /hello.txt',\
			'./ls /','./ln /hello.txt /hello.lnk/']
			
expected = "Should produce error when trying to hard link to a directory. It should also error when the new link is a given as a directory"
