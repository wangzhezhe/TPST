# test this on elf cluster
# using this dir /home1/zw241/Software/swift-0.96.2/swift-tutorial/testtime
# simulation is located in this dir swift-tutorial/app
# refer to this tutorial to set up the init env http://swift-lang.org/swift-tutorial/doc/tutorial.html

import datetime
import time
import os

current_time1 = datetime.datetime.now()
# execute the seift chained operation
os.system('swift chain.swift')
current_time2 = datetime.datetime.now()

print (current_time2 - current_time1)