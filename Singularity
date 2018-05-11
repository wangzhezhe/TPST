Bootstrap: docker
From: ubuntu:16.04

%post
    apt-get -y update
    apt-get -y install fortune cowsay lolcat
    apt-get -y install build-essential autoconf libtool pkg-config
    apt-get -y install libgflags-dev libgtest-dev
    apt-get -y install clang libc++-dev
    apt-get -y install git curl
    git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
    cd grpc
    git submodule update --init
    make
    make install
    

%environment
    export LC_ALL=C
    export PATH=/usr/games:$PATH

%runscript
    pwd
    ls
    fortune | cowsay | lolcat
