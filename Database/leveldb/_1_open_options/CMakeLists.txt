cmake_minimum_required(VERSION 3.13)
project(LevelDBTest)

set(CMAKE_CXX_STANDARD 11)

add_executable(LevelDBTest main.cc)
target_link_libraries(LevelDBTest leveldb.a)
target_link_libraries(LevelDBTest pthread -lm -ldl)