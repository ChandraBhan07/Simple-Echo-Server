cmake_minimum_required(VERSION 3.15)
project(SimpleServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client src/client)
target_link_libraries(client ws2_32)

add_executable(server src/server)
target_link_libraries(server ws2_32)
