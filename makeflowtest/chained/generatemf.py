
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])


originalstr='''

sim.out: ../app/simulate.sh prev.out
    /bin/bash ../app/simulate.sh --timesteps 1 --range 100 --nvalues 5 --log off > sim.out

ana.out: sim.out ../app/analysis.sh
    /bin/bash ../app/analysis.sh sim.out > ana.out

'''




file = open("chained.mf","a")

for x in range(2,pairnum+1): 
    simout='sim'+str(x)+'.out'
    anaout='ana'+str(x)+'.out'
    prevout = 'ana'+str(x-1)+'.out'
    #replace the mf str and write into new file
    #print simout
    ndata=originalstr.replace('sim.out',simout)
    ndata=ndata.replace('ana.out',anaout)
    ndata=ndata.replace('prev.out',prevout)
    print ndata
    file.write(ndata)


file.close()
