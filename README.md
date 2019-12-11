This project provides the message communication pattern based on the pub/sub interfaces. A similar [project](https://github.com/wangzhezhe/MMServer) provides the message communication pattern based on the shared space and the message queue.

### running on caliburn

if run the server on caliburn

load python and virtual env

```
module load python/3.6.3

source /project1/parashar-001/zw241/GridMoving/gridsim/bin/activate
```

load the env for grpc 

```
source ~/.grpc
```

load mpi

```
module unload openmpi/1.10.1
module load mpich
```


```
#the path of the graph relevent binary
export PATH=$PATH:/home1/zw241/Software/grpcinstall/bin

#the path of the proton binary
export PATH=$PATH:/home1/zw241/Software/grpc/third_party/protobuf/src

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/home1/zw241/Software/protoinstall/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/home1/zw241/Software/grpcinstall/lib/pkgconfig

#set the path of the dynamic library
export LD_LIBRARY_PATH=/home1/zw241/Software/grpc/libs/opt
```

get the code for caliburn env


```
module unload openmpi/1.10.1
module load mpich
source ~/.grpc

cd /project1/parashar-001/zw241/software/eventDrivenWorkflow

git fetch --all
git reset --hard origin/caliburnEnv
git pull origin caliburnEnv:caliburnEnv
```

make the publishclient first 

then make the server 

then make the 

```
rm -rf multinodeip
mpirun -n 1 ./workflowserver 1500 eno1 50052 1
```





