
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

with open ("triguresim.json", "r") as myfile:
    sim=myfile.read()

#with open ("trigureana.json", "r") as myfile:
#    ana=myfile.read()



# copy trigure1ana.json

for x in range(2,pairnum+1):

    simout="sim"+str(x)+".out"
 #   anaout="ana"+str(x)+".out"


    simevent="T"+str(x)+"SIM_FINISH"
 #   anaeventcurr="T"+str(x)+"ANA_FINISH"
 
    newsim=sim.replace("T1SIM_FINISH",simevent)
 #   newana=ana.replace("T1SIM_FINISH",simevent)
 #   newana=newana.replace("T1ANA_FINISH",anaeventcurr)


    
    newsim=newsim.replace("sim1.out",simout)
 #   newana=newana.replace("ana1.out",anaout)
 #   newana=newana.replace("sim1.out",simout)

    filenamesim="trigure_"+str(x)+"sim.json"
  #  filenameana="trigure_"+str(x)+"ana.json"
    fsim= open(filenamesim,"w+")
   # fana= open(filenameana,"w+")
    fsim.write(newsim)
    #fana.write(newana)
    fsim.close()
    #fana.close()

