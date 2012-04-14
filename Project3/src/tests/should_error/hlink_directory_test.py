output = "./tests/should_error/hlink_dir.result"

description = "Test that hard linking directories results in an error"

options = "estop rebuild"

execlist = ['./mkdir /home',\
			'./ln /home /home_link.lnk',\
			'./ls /']
			
expected = "Should produce error when trying to hard link directory"
