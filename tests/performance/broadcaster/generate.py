
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys
import os

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])

os.system("rm *_*")

# copy trigure1sim.json

with open ("simtemplate", "r") as mysim:
    sim=mysim.read()

#newsim=sim.replace("NUM",str(pairnum))

#print newsim

#fsim= open("sim.json","w+")

#fsim.write(newsim)

#fsim.close()

#with open ("trigureana.json", "r") as myfile:
#    ana=myfile.read()


for x in range(1,pairnum+1):

    simout="sim"+str(x)+".out"
    
    newsim=sim.replace("sim1.out",simout)
    

    filenamesim="trigure_"+str(x)+"sim.json"

    fsim= open(filenamesim,"w+")
 
    fsim.write(newsim)

    fsim.close()


