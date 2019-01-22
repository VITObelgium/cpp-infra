import os
import re
from glob import glob

files = glob('**/*.prl', recursive=True)
libpattern = re.compile(r"[-][l]\S*\s")
libdirpattern = re.compile(r"[-][L]\S*\s")

for f in files:
    openedfile = open(f, "r")
    builder = ""
    for line in openedfile:
        if line.startswith("QMAKE_PRL_LIBS = "):
            line = libpattern.sub(" ", line)
            line = libdirpattern.sub("", line)
            line = line.replace("\\\\", "/")
            builder += line
        else:
            builder += line

    new_file = open(f, "w")
    new_file.write(builder)
    new_file.close()
    print("processed: " + f)