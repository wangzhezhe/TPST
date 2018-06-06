#!/bin/bash

g++ -fopenmp -std=c++11 -fopenmp `pkg-config --cflags protobuf grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src  -c -o notifyserver.o notifyserver.cc

g++ -fopenmp -std=c++11 -fopenmp `pkg-config --cflags protobuf grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src -c -o getip.o ../utils/getip/getip.cpp

g++ -fopenmp -std=c++11 -fopenmp `pkg-config --cflags protobuf grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src  -c -o testNotifyServer.o testNotifyServer.cc

g++ -fopenmp workflowserver.pb.o workflowserver.grpc.pb.o notifyserver.o testNotifyServer.o getip.o -L/usr/local/lib -L/home1/zw241/Software/protoinstall/lib -L/home1/zw241/Software/grpc/libs/opt/protobuf -L/home1/zw241/Software/grpcinstall/lib `pkg-config --libs protobuf grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -lpthread -lgpr -luuid -o notifyserver