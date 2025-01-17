cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
    GroupChatTuringTest-Server
    
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let's ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

message(NOTICE "")
message(NOTICE "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(NOTICE "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
message(NOTICE "CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message(NOTICE "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fdiagnostics-color=always -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fdiagnostics-color=always -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# Place executable in "bin" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)  
set(SRC_DIR_PATH ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(lib)

set(EXECUTABLE_NAME "server")

add_executable(
    ${EXECUTABLE_NAME}

    # Add source files here
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ServerLogic/ServerLogic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ServerLogic/MessageHandlers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lobby/Lobby.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIdGenerator/UniqueIdGenerator.cpp
)

target_include_directories(
    ${EXECUTABLE_NAME}

    # Add dirs with header files here
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PerSocketData/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ServerLogic/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lobby/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueIdGenerator/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MsgTypeEnum/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/nlohmann/
)

target_link_libraries(
    ${EXECUTABLE_NAME}
    
    uWebSockets
    -lzmqpp
    -lzmq
)
