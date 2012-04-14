output = "tests/fsck_tests/fsck1_test.result"

description = "Tests fsck."

options = "EStop rebuild"

execlist = ['cat ./tests/64_bytes.txt | ./tee /config.txt',\
		'./mkdir /home',\
		'./mkdir /scripts',\
		'./mkdir /home/user1',\
		'./mkdir /home/user1/school',\
		'./mkdir /home/user1/school/5103',\
		'./mkdir /home/user1/school/4131',\
            	'./ln /config.txt /home/user1/myconfig.txt -s',\
            	'cat ./tests/64_bytes.txt | ./tee /home/user1/school/schedule.txt',\
		'cat ./tests/64_bytes.txt | ./tee /home/user1/school/4131/hw1.txt',\
            	'./crpt 2 4',\
		'./fsck -silent',\
		'./fsck -silent']

expected = " Fsck should find 10 IndexNode link errors and 3 block errors and correct them."
