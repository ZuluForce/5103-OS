output = "./tests/link_tests/symdirectory.result"

options= "estop rebuild"

execlist = ['./mkdir /home/','./mkdir /home/Planeman/',\
			'echo "Hello" | ./tee /hello.txt',\
			'./ln /home/Planeman/ /home_link.txt -s',\
			'./ln / /home/Planeman/root.lnk -s',\
			'./ln /hello.txt /home/Planeman/hello.lnk -s',\
			'./ls /','./ls /home','./ls /home/Planeman',\
			'./cat /hello.txt','./cat /home/Planeman/hello.lnk']
			
expected = "All the symlinks should work."
