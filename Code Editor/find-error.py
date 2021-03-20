#!/usr/bin/python

import sys, re

pattern_reg = re.compile('^.+:(\d+):(\d+): (error|warning):')

first_error = -1;
msg_error = "";
first_warning = -1;
msg_warning = "";

for line in sys.stdin:
    line = line.rstrip('\n')
    m = re.search(pattern_reg, line)
    if (m):
        # in the pattern, each set of parentheses represents a "group"
        line_number = int(m.group(1))
        err = m.group(3)
        if err == "error" :
            if first_error < 0 or line_number < first_error :
                first_error = line_number
                msg_error = line
        else :
            if first_warning < 0 or line_number < first_warning :
                first_warning = line_number
                msg_warning = line

if first_error >= 0 :
    print(msg_error)
elif first_warning >= 0 :
    print(msg_warning)
else :
    print("")
