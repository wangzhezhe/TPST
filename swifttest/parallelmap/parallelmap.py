# test this on elf cluster
# using this dir /home1/zw241/Software/swift-0.96.2/swift-tutorial/testtime
# simulation is located in this dir swift-tutorial/app
# refer to this tutorial to set up the init env http://swift-lang.org/swift-tutorial/doc/tutorial.html
# export PATH=$PATH:/home1/zw241/Software/swift-0.96.2/bin
# source setup.sh  # You must run this with "source" !
# module load java
# use interactive mode to get a dedicated nodes(salloc -N 1 bash)
# modify this, chain mode should be in the following case sm1->ana1->sim2->ana2->sim3->ana3->sim4->ana4...

import datetime
import time
import os

os.system('rm ./output/*')
os.system('touch ./output/init.out')

#for x in range(0,5):
current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system('swift parallelmap.swift')
current_time2 = datetime.datetime.now()
time=(current_time2 - current_time1)

print time
#print sum/5