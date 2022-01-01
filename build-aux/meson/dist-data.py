#!/usr/bin/env python3

import os
import shutil
import subprocess

from pathlib import PurePath

stylesheets = [
    'src/stylesheet/base.css',
    'src/stylesheet/base-hc.css',
    'src/stylesheet/defaults-light.css',
    'src/stylesheet/defaults-dark.css',
]

references = [
    'doc/libadwaita-1',
]

sourceroot = os.environ.get('MESON_SOURCE_ROOT')
buildroot = os.environ.get('MESON_BUILD_ROOT')
distroot = os.environ.get('MESON_DIST_ROOT')

for stylesheet in stylesheets:
    stylesheet_path = PurePath(stylesheet)
    src = PurePath(sourceroot, stylesheet_path.with_suffix('.scss'))
    dst = PurePath(distroot, stylesheet_path)
    subprocess.call(['sassc', '-a', '-M', '-t', 'compact', src, dst])

for reference in references:
    src_path = os.path.join(buildroot, reference)
    if os.path.isdir(src_path):
        dst_path = os.path.join(distroot, reference)
        shutil.copytree(src_path, dst_path)
