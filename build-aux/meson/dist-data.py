#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys

from pathlib import PurePath

sourceroot = os.environ.get('MESON_SOURCE_ROOT')
distroot = os.environ.get('MESON_DIST_ROOT')

stylesheet_path = PurePath('src/stylesheet/gtk.css')
src = PurePath(sourceroot, stylesheet_path.with_suffix('.scss'))
dst = PurePath(distroot, stylesheet_path)
subprocess.call(['sassc', '-a', '-M', '-t', 'compact', src, dst])
