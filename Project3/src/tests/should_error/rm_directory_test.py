output = "./tests/should_error/rm_directory.result"

description = "Test that trying to remove a directory generates an error"

options = "rebuild"

execlist = ['./mkdir /home','./rm /home',\
			'./ls /',\
			'./ln /home /home_link.lnk -s',\
			'./ls /',\
			'./rm /home_link.lnk',\
			'./ls /']
			
expected = """The first removal should fail because it is on a directory.
The second should work because it is removing a symbolic link to a directory."""
