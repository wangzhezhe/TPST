
# dir on caliburn /home1/zw241/Software/cctools-source/cctoolinstall/testmakeflow

import datetime
import time
import os

os.system("rm *.out")

current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system('../../bin/makeflow -T local chain.mf')
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)