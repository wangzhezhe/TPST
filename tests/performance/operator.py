import datetime
import time
import os


current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system('./operator publish INIT')
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)