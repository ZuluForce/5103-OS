output = "./tests/should_error/too_long.result"

options = "rebuild"

execlist = ['echo "Hello" | ./tee /a_very_long_filename.txt']

expected = "Should give an error about a bad filename"
