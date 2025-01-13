FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    g++ \
    libz-dev \
    cmake \
    git \
    bash \
    python3 \
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

COPY . /app
WORKDIR /app

RUN bash build.sh

EXPOSE 12345

# This command runs your application, comment out this line to compile only
CMD ["./bin/server"]
