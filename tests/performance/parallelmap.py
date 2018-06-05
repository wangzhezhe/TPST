import datetime
import time
import os
import sys

os.system("rm *.out")

current_time1 = datetime.datetime.now()
# execute the seift chained operation
dirIndex=sys.argv[1]
if int(dirIndex)==0:
    dirName=' ./parallelmap/'
    command = './eventNotify'+ dirName + ' 128'
else:
    dirName=' ./parallelmap'+dirIndex+'/'
    command = './eventNotify'+ dirName + ' 128'
print command
os.system(command)
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)