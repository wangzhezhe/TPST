import datetime
import time
import os
import sys

os.system("rm *.out")

dirIndex=sys.argv[1]
if int(dirIndex)==0:
    dirName=' ./broadcaster/'
    command = './eventNotify'+ dirName + ' 1024'
else:
    dirName=' ./broadcaster'+dirIndex+'/'
    command = './eventNotify'+ dirName + ' 1024'
print command

current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system(command)
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)