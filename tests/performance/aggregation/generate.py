
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys
import os

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])

os.system("rm trigure_*")

# copy trigure1sim.json

with open ("triguresim1.json", "r") as myfile:
    sim=myfile.read()

with open ("trigureana_templeate", "r") as myfile:
    ana=myfile.read()

simoutlist=''

# copy trigure1ana.json

for x in range(2,pairnum+1):

    simout="sim"+str(x)+".out"

    simoutlist=simoutlist+" "+simout

    
    newsim=sim.replace("sim1.out",simout)
    

    filenamesim="trigure_"+str(x)+"sim.json"

    fsim= open(filenamesim,"w+")
   
    fsim.write(newsim)
    fsim.close()



fana= open("trigureana.json","w+")
newana=ana.replace("sim.out",simoutlist)
newana=newana.replace("NUM",str(pairnum))
fana.write(newana)
print newana
fana.close()


