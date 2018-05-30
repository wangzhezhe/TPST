
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])


originalstr='''

sim.out: ../app/simulate.sh
    /bin/bash ../app/simulate.sh --timesteps 1 --range 100 --nvalues 5 --log off > sim.out

'''

anastr='''
ana.out: sim.out ../app/analysis.sh
    /bin/bash ../app/analysis.sh sim.out > ana.out
'''



file = open("aggregation.mf","w")

simputList=' '
for x in range(1,pairnum+1): 
    simout='sim'+str(x)+'.out'
    simputList = simputList+simout+' '
    ndata=originalstr.replace('sim.out',simout)

    print ndata
    file.write(ndata)


print simputList

newanstr=anastr.replace('sim.out',simputList)
file.write(newanstr)

file.close()
