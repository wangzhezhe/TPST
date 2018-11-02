
import timeit
import os

# time 1
# start pub/sub server and operator

startsim = timeit.default_timer()
print ("start time")
print (startsim)

os.system("python ./semiEulerGrid.py")

#start operator

# time 2

# simulation (record detect time)


# analysis record finish time