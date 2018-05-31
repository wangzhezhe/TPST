
#generate series of coupling task pairs based on chanin.mf

#pairnum=3

import sys
import os

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])

os.system("rm trigure_*")


with open ("trigureana.json", "r") as myfile:
    ana=myfile.read()

simoutlist=''

# copy trigure1ana.json

for x in range(2,pairnum+1):

    #simout="sim"+str(x)+".out"
    anaout="ana"+str(x)+".out"
    
    #newana=ana.replace("sim1.out",simout)
    newana=ana.replace("ana1.out",anaout)

    filenameana="trigure_"+str(x)+"ana.json"

    fsim= open(filenameana,"w+")
   
    fsim.write(newana)
    fsim.close()



