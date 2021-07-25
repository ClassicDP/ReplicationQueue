cmake_minimum_required(VERSION 3.19)
project(untitled)

set(CMAKE_CXX_STANDARD 14)

add_executable(untitled main.cpp Cluster.cpp Cluster.h types.h QueueFile.cpp QueueFile.h StackList.cpp StackList.h DynamicArray.cpp DynamicArray.h)