[TODO]

change grreter into dynamic way (load ip itself)

make it run on caliburn (source ~/.grpc before compilling)

workflow server subscribe parameter times, should be loaded by request

[depedencies]:
protobuf
(https://blog.jeffli.me/blog/2016/12/08/install-protocol-buffer-from-source-in-centos-7/)
https://blog.csdn.net/Programmer_H/article/details/8890800

grpc(for protobuf of grpc, use inner version of grpc by 3.4.0)

add the directory containing `grpc.pc'
to the PKG_CONFIG_PATH environment variable


sudo apt-get install uuid-dev

make the pubsub part thread safety
(https://www.linkedin.com/pulse/simple-way-implement-concurrent-mapset-c-huan-xia/)

integrate pubsub part with rpc server (ok for testpublishsubscribe)

[experiments]

1> ok finish registering .json file automatically and then calculate the time 
by defining extra compile parameter to control this

2> add a python script to create any number of event triggure files

3> add a python script to calculate the time (learn how to draw the whiskers plot client number: 10 20 30 ...)

[bugs]

when publish before subscribe, do nothing

event1 register e1 e2 than start, if modify trigure, and add e3, the obvious client is still listening to the redisclient
if there are event published, the command will be started moutiple times

when doing the scale testing, it's ok to output the data into terminal but faile to output it by using redirect 2>&1 ???
some log may lose in multi thread cases