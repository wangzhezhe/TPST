#!/bin/bash
rm pubsub.o test.o test

g++ -g -std=c++11 -fopenmp `pkg-config --cflags protobuf grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src -c -o pubsub.o pubsub.cc

g++ -g -std=c++11 -fopenmp `pkg-config --cflags protobuf grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src -c -o test.o test.cc

g++ -g -fopenmp split.o pubsub.o test.o -L/usr/local/lib -L/usr/local/lib -L/home1/zw241/Software/protoinstall/lib -L/home1/zw241/Software/grpc/libs/opt/protobuf -L/home1/zw241/Software/grpcinstall/lib `pkg-config --libs protobuf grpc++ grpc` -I/home1/zw241/Software/grpc/third_party/protobuf/src -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -o test
