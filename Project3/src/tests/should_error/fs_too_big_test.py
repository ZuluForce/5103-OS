output = "./tests/should_error/fs_too_big.result"

description = "Check that we get an error when creating a file too big to fit in the indoe's direct blocks. Note that if we implement single/double/triple indirect blocks this test will no longer error assuming there is sufficient space."

options = "rebuild"

execlist = ['cat ./tests/moby.txt ./tests/moby.txt | ./tee /moby.txt']

expected = """Should produce an error"""
