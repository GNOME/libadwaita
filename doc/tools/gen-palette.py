#!/usr/bin/env python3

import re

SCSS = '../../src/stylesheet/_palette.scss'
LIGHT = set(['light-1', 'light-2'])
DARK = set(['dark-4', 'dark-5'])

colors = []
regex = re.compile('^\$(\w+)_([1-5]):\s*(#\w+);$')

# Parse colors
with open(SCSS, 'r') as scss:
    for line in scss:
        match = regex.match(line)
        if match == None:
            continue
        name = match.group(1)
        level = match.group(2)
        value = match.group(3)

        colors.append((name, level, value))

# Generate table
print('<table>')
print('  <tr>')
print('    <th></th>')
print('    <th>Name</th>')
print('    <th>Value</th>')
print('  </tr>')

for name, level, color in colors:
    variable = '--{}-{}'.format(name, level)

    classes = ''
    if variable in LIGHT:
        classes += ' light'
    if variable in DARK:
        classes += ' dark'

    print('  <tr>')
    print('    <td><div class="color-pill{}" style="background-color: {}"/></td>'.format(classes, color))
    print('    <td><tt>{}</tt></td>'.format(variable))
    print('    <td><tt>{}</tt></td>'.format(color))
    print('  </tr>')

print('</table>')

