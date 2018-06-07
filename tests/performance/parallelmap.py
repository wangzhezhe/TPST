import datetime
import time
import os
import sys

os.system("rm *.out")

dirIndex=sys.argv[1]
numtoStop=sys.argv[2]


if int(dirIndex)==0:
    dirName=' ./parallelmap/'
    command = './eventNotify'+ dirName + ' ' + str(numtoStop)
else:
    dirName=' ./parallelmap'+dirIndex+'/'
    command = './eventNotify'+ dirName + ' '+str(numtoStop)
print command

current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system(command)
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)