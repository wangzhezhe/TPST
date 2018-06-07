# start the operator firstly(before running the notify scripts) when doing testing!
import datetime
import time
import os
import sys

os.system("rm *.out")

current_time1 = datetime.datetime.now()
dirIndex=sys.argv[1]
numtoStop=sys.argv[2]


if int(dirIndex)==0:
    dirName=' ./aggregation'
    command = './eventNotify'+' '+ dirName + ' ' +str(numtoStop)
else:
    dirName=' ./aggregation'+dirIndex+'/'
    command = './eventNotify'+ ' '+dirName + ' '+str(numtoStop)
print command
os.system(command)
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)