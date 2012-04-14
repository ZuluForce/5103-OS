output = "tests/link_tests/symlink1_test.result

description = "To come later"

options = "EStop rebuild"

execlist = ['./cat tests/64_bytes.txt | ./tee /file64.txt',\
            './ln /64file.txt /link1.txt -s',\
            './mkdir /home',\
            './ln /64file.txt /home/link2.txt -s',\
            './ls /','./ls /home',\
            './cat /64file.txt','./cat /link1.txt','./cat /home/link2.txt']

expected = "Awesomeness"
