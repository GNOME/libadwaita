#!/usr/bin/env python3

import os, sys, re

infile = sys.argv[1]
outfile = sys.argv[2]
icon_paths = sys.argv[3:]

icons = {}

for path in icon_paths:
    files = os.listdir(path)

    for f in files:
        if f.endswith('.svg') or f.endswith('.gpa'):
            icon_name = f[:-4]
            icons[icon_name] = os.path.join(path, f)

with open(infile, 'r') as f:
    contents = f.read();

matches = re.finditer('@icon:(.*?)@', contents)

for match in reversed(list(matches)):
    icon_name = match[1]
    start = match.start(0)
    end = match.end(0)

    if not icon_name in icons:
        print('Unknown icon: {}'.format(icon_name))
        exit(1)

    with open(icons[icon_name], 'r') as f:
        svg = f.read()

    contents = contents[:start] + svg + contents[end:]

with open(outfile, 'w') as f:
    f.write(contents)
