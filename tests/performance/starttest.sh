#!/bin/bash

cd ..

make clean

make

cp ./eventNotify ./performance

cd ./performance

#./eventNotify ./chain/
