import os
import re
from glob import glob

files = [y for x in os.walk('.') for y in glob(os.path.join(x[0], '*.prl'))]
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