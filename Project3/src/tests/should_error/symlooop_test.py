output = "./tests/should_error/symloop.result"

description = "Testing that a symbolic link loop is properly detected and handled (by error)."

options = "rebuild"

execlist = ['./mkdir /home',\
			'./ln /home/bad_link2.lnk /home/bad_link.lnk -s',\
			'./ln /home/bad_link.lnk /home/bad_link2.lnk -s',\
			'./ls /','./ls /home',\
			'./ls /home/bad_link.lnk',\
			'./cat /home/bad_link.lnk']
			
			
expected=\
"""The pair of symbolic links point to one another and since cat wants the pointed to contents
the dereferencing will start an infinite loop which should be detected after Kernel::hopTresh
dereferences."""
