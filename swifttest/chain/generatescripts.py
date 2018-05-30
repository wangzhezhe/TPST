import datetime
import time
import os
import sys

if len(sys.argv)!=2:
    print './generate.py <pairnum>'
    sys.exit(0)  

pairnum=int(sys.argv[1])

originstr='''file simout <single_file_mapper; file=strcat("output/simout.out")>;
simout = simulation(steps,range,values,prev);

file anaout <single_file_mapper; file=strcat("output/anaout.out")>;
anaout = analyze(simout);'''

file=open('chain.swift', 'a')

for x in range(2,pairnum+1):
    simout='simout'+str(x)
    anaout='anaout'+str(x)
    anaoutprev='anaout'+str(x-1)
    nstr=originstr.replace('simout',simout)
    nstr=nstr.replace('anaout',anaout)
    nstr=nstr.replace('prev',anaoutprev)
    file.write('\n\n')
    file.write(nstr)

file.close()

