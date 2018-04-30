
import re
#load file

fname="./stdout.log"
#fname="./temp.txt"
f=open(fname,"r")
#scan line by line
line = f.readline()
start=[]
time=[]
avgt=0.0
mint=9999999999
maxt=0.0
count=0
sum=0.0
# use the read line to read further.
# If the file is not empty keep reading one line
# at a time, till the file is empty
while line:
    if line.find("start sec")!=-1 and line.find("start nsec")!=-1:
        #parse start, get sec and nsec
        #print line
        #extract integer
        start = re.findall(r'\d+',line)


    if line.find("finish sec")!=-1 and line.find("finish nsec")!=-1:
        #print line
        finish = re.findall(r'\d+',line)
        count=count+1
        # Read a line.
        time = (float(finish[-2])-float(start[-2])+(float(finish[-1])-float(start[-1]))/1000000)
        sum=sum+time
        # print time
        if time<mint:
            mint=time
        if time > maxt:
            maxt=time
        avgt=sum/count
        
    line = f.readline()


print 'count %d min %f max %f avg %f' % (count ,mint ,maxt,avgt)