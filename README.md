# !!! cmake build problems
If cmake displays a weird problem with CMakeCache and something about current directory being different than some other directory just uncomment the line<br>
```RUN rm -r build/*```<br>
in the Dockerfile

# The server is set to run on port: 12345 (it is hardcoded for now)

# Build and run with docker
docker compose up --build

# Run cmake config
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug<br>
OR<br>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release<br>

# Build the project
cmake --build build
