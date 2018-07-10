If run build and run it on Titan, jump to login node to use inactive mode

```
qsub -I -X -l walltime=00:30:00 -A CSC103 -l nodes=1
module unload PrgEnv-pgi
module load PrgEnv-gnu
```

For titan, modifing the link path (the stdgcc in cray/gcc-libs satisfy the requirements)
the dir in root path is grpc  grpcinstall  sharedlib, grpc is the source file of original grpc repo, grpcinstall is the relative files after executing make install for grpc, sharedlib is the dynamic library satisfy the requirements of grpc, on Titan, the library /opt/cray/gcc-libs/libstdc++.so.6 satisfy the requiremnts

execute following command to set the environment

```
export ROOT_PATH=/lustre/atlas/scratch/zw241/csc103/Software
export PATH=$PATH:$ROOT_PATH/grpc/bins/opt
export PATH=$PATH:$ROOT_PATH/grpc/third_party/protobuf/src

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$ROOT_PATH/grpc/third_party/protobuf

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$ROOT_PATH/grpc/libs/opt/pkgconfig

export LD_LIBRARY_PATH=$ROOT_PATH/grpc/third_party/protobuf/src

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/atlas/scratch/zw241/csc103/Software/grpc/libs/opt

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/atlas/scratch/zw241/csc103/Software/sharedlib/gcc-libs
```
then modify the Makefile lightly

sample Makefile to compile helloword example

```
ROOT_PATH = /lustre/atlas/scratch/zw241/csc103/Software
LD_PATH_PROTO = $(ROOT_PATH)/grpc/third_party/protobuf/src
LD_PATH_GRPC = $(ROOT_PATH)/grpcinstall/include
HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++ -I$(LD_PATH_PROTO) -I$(LD_PATH_GRPC)
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib -I$(LD_PATH) `pkg-config --libs protobuf grpc++ grpc`\
           -lgrpc++_reflection\
           -ldl
else
LDFLAGS += -L/usr/local/lib -L$(ROOT_PATH)/grpcinstall/lib -L$(ROOT_PATH)/grpc/libs/opt/protobuf -I$(LD_PATH) `pkg-config --libs protobuf grpc++ grpc`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
......

```

then load the warprun to execute the excutable file

```
module load dynamic-link
module load python wraprun
wraprun -n 1 <executable>
```

//set the env of the grpc
source ~/.grpc

//set the color of the dir
source ~/.bashrc

On titan, if you want to use multimode, you should subscribe those nodes firstly, the number of nodes should be modified here `qsub -I -X -l walltime=00:30:00 -A CSC103 -l nodes=1`



This is the exmple of running one server one client case on Titan

```
## build publishclient firstly befor building other components

## server

cd /lustre/atlas/scratch/zw241/csc103/Software/eventDrivenWorkflow/tests/performance
rm -rf multinodeip


## Load env
source ~/.grpc
module load dynamic-link
module load python wraprun


## Run workflow servers
## ./workflowserver <subscribe period time> <number of the nodes> <network interfaces>
wraprun  -n 1 ./workflowserver 1500 1 ipogif0   > notify1s1c.txt 2>&1 &

## client

cd /lustre/atlas/scratch/zw241/csc103/Software/eventDrivenWorkflow/tests/performance

## Load env
source ~/.grpc
module load dynamic-link
module load python wraprun

## Run workflow client
wraprun -n 1 ./eventNotify 128 10 ipogif0 50052 1

```

