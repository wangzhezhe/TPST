# start the operator firstly(before running the notify scripts) when doing testing!
import datetime
import time
import os

os.system("rm *.out")

current_time1 = datetime.datetime.now()
dirIndex=sys.argv[1]
if int(dirIndex)==0:
    dirName=' ./aggregation/'
    command = './eventNotify'+ dirName + ' 128'
else:
    dirName=' ./aggregation'+dirIndex+'/'
    command = './eventNotify'+ dirName + ' 128'
print command
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)