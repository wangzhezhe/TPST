
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])

# load file content to str
with open('parallelmap_template.mf', 'r') as myfile:
  data = myfile.read()

print data

file = open("parallelmap.mf","w")

for x in range(1,pairnum): 
    simout='sim'+str(x)+'.out'
    anaout='ana'+str(x)+'.out'
    #replace the mf str and write into new file
    #print simout
    ndata=data.replace('sim.out',simout)
    ndata=ndata.replace('ana.out',anaout)
    file.write(ndata)


file.close()
