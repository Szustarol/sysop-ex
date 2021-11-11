#!/usr/bin/python3

import sys
import os 

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, "r") as _inp:
    with open(output_file, "r") as _out:
        out = _out.read()
        inp = _inp.read()
        inp = inp.replace("\n", " ")
        if inp in out:
            print("\033[0;32m{} contains {}.".format(output_file, input_file))
        else:
            print("\033[1;31m{} DOES NOT contain {}.".format(output_file, input_file))
        os.system("tput sgr0")
