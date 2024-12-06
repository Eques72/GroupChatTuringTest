FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    g++ \
    libz-dev \
    cmake \
    git \
    bash

COPY . /app

WORKDIR /app

RUN bash build.sh

EXPOSE 12345

WORKDIR /app/bin

# This command runs your application, comment out this line to compile only
CMD ["./server"]
