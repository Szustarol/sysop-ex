import random
import string

nLines = int(input(""))
nChars = int(input(""))
output = (input(""))

with open(output, 'w') as f:
    while nLines > 0:
        line = ""
        for i in range(0, nChars):
            line += random.choice(string.ascii_letters)
        line += "\n"
        f.write(line)
        nLines-=1

