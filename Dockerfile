FROM ubuntu:latest

# TODO Either create a script that cheks for and creates python venv or somehow get the pip installing correctly

RUN apt-get update && apt-get install -y \
    g++ \
    libz-dev \
    cmake \
    git \
    bash \
    python3 python3-pip python3-dev \
    libzmq3-dev \
    pkg-config

RUN git clone https://github.com/zeromq/zmqpp.git /tmp/zmqpp && \
    cd /tmp/zmqpp && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/zmqpp

RUN pip3 install pyzmq

COPY . /app
WORKDIR /app

RUN bash build.sh

EXPOSE 12345

# This command runs your application, comment out this line to compile only
CMD ["./bin/server"]
