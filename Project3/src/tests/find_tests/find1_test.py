output = "tests/find_tests/find1_test.result"

description = "Tests the recursive capabilities of find."

options = "estop rebuild"

execlist = ['cat ./tests/64_bytes.txt | ./tee /config.txt',\
		'./mkdir /home',\
		'./mkdir /scripts',\
		'./mkdir /home/user1',\
		'./mkdir /home/user1/school',\
		'./mkdir /home/user1/school/5103',\
		'./mkdir /home/user1/school/4131',\
            	'./ln /config.txt /home/user1/myconfig.txt -s',\
		'./ln /bin /home/user1/mybin -s',\
            	'cat ./tests/64_bytes.txt | ./tee /home/user1/school/schedule.txt',\
		'cat ./tests/64_bytes.txt | ./tee /home/user1/school/5103/hw1.txt',\
		'cat ./tests/64_bytes.txt | ./tee /home/user1/school/5103/hw2.txt',\
		'cat ./tests/64_bytes.txt | ./tee /home/user1/school/4131/hw1.txt',\
            	'./find /',\
		'./find /home/user1/school',\
            	'./find /home/user1/school/5103/hw1.txt']

expected = "What find should display for each of the three calls (one on root directory, one on specific directory, and one on a file."
