output = "./tests/link_tests/lcreate.result"

description = "Show that a symlink is not tied to a file becase we can create the file it is supposed to point to after it is actually created."

options = "estop rebuild"

ignore_error = ['./ls /home/Planeman',\
				'./ls /my_books.lnk -r',\
				'./cat /my_books.lnk/moby.lnk']

execlist = ['./mkdir /home',\
			'./ln /home/Planeman/books /my_books.lnk -s',\
			'./ls /','./ls /home',\
			'./ls /home/Planeman',\
			'./ls /my_books.lnk -r',\
			'./mkdir /home/Planeman',\
			'./mkdir /home/Planeman/books',\
			'./ln /moby.txt /my_books.lnk/moby.lnk -s',\
			'./ls /home/Planeman/',\
			'./ls /home/Planeman/books',\
			'./ls /my_books.lnk/',\
			'./cat /my_books.lnk/moby.lnk',\
			'cat ./tests/moby.txt | ./tee /moby.txt',\
			'./ls /',\
			'./cat /moby.txt', './cat /my_books.lnk/moby.lnk',\
			'./cat /home/Planeman/books/moby.lnk']

expected = "I'm not sure what I even wrote"
