#!/bin/bash
rm pubsub.o test.o test

g++ -g -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o pubsub.o pubsub.cc

g++ -g -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o test.o test.cc

g++ -g pubsub.o test.o -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -o test
