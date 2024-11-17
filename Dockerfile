FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    g++ \
    libz-dev \
    cmake \
    git

COPY . /app

WORKDIR /app

# Uncomment if cmake has some random problems with CMakeCache directory being different than some other directory
# Essentially a clean build, since "cmake --target clean" is kinda weird for me
# RUN rm -r build/*

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

EXPOSE 12345

# This command runs your application, comment out this line to compile only
CMD ["./bin/server"]
