
import sys
fa=open("./trigureA.json","r")
fileAContent=fa.read()
#print fileAContent
if len(sys.argv)!=2:
    print "<script> <number>"
    exit()
n=int(sys.argv[1])
for num in range(0,n):
    filename="test_trigure"+str(num)+".json"
    print filename
    f = open(filename, 'w')
    f.write(fileAContent)
    f.close()


