
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

with open ("trigureanatemplate", "r") as myana:
    ana=myana.read()

#newsim=sim.replace("NUM",str(pairnum))

#print newsim

#fsim= open("sim.json","w+")

#fsim.write(newsim)

#fsim.close()

#with open ("trigureana.json", "r") as myfile:
#    ana=myfile.read()


for x in range(1,pairnum+1):

    anaout="ana"+str(x)+".out"
    
    newana=ana.replace("ana1.out",anaout)
    

    filenameana="trigure_"+str(x)+"ana.json"

    fsim= open(filenameana,"w+")
 
    fsim.write(newana)

    fsim.close()


