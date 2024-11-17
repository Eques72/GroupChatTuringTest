# Build and run with docker
docker compose up --build

# Run cmake config
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug<br>
OR<br>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release<br>

# Build the project
cmake --build build
