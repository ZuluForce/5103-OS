output = "link.test.result"
options = "clean EStop"
execlist = ['./mkfs filesys.dat 256 20',\
		'./mkdir /school/',\
		'cat ../tests/64_bytes.txt | ./tee /64_byte_file.txt',\
		'./ln /64_byte_file.txt /school/link_64.txt',\
		'./cat /64_byte_file.txt',\
		'./cat /school/link_64.txt',]

expected = """/* ---- No expected output for this test ---- */""""
