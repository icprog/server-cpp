cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
link_libraries("-lmysqlclient_r")

set(SOURCE_FILES main.cpp Util/libc/code.c Util/libc/process.c Util/libc/io.c Util/libc/socket.c Util/libc/ipc.c Util/libc/statistics.c User/User.cpp User/User.h activity.cpp Util/libc/db.c Util/libc/db.h Util/DBUtil.cpp Util/DBUtil.h)
add_executable(server ${SOURCE_FILES})