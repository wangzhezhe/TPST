import datetime
import time
import os

os.system("rm *.out")

current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system('./eventNotify ./aggregation/')
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)