__author__ = 'Lord'

import shutil
import os
import sys

print("\n### process post build")
# print("[debug] sys args: ")
# print(sys.argv)

if len(sys.argv) != 2: exit(-1)

dstDir = sys.argv[1]

if sys.platform == 'darwin':
    bundlePath = 'JekyllPoster.app/Contents/MacOS'
    dstDir = os.path.join(dstDir, bundlePath)

print("Build path: " + dstDir)

if not os.path.isdir(dstDir):
    exit(-2)

thisPath = os.path.realpath(__file__)
thisDir = os.path.dirname(thisPath)
srcFile = os.path.join(thisDir, 'gen_poster.py')

print("copy the following file to build path:\n%s\n" % srcFile)

try:
    shutil.copy(srcFile, dstDir)
except:
    print("")
    exit(-3)

exit(0)