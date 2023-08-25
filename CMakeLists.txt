cmake_minimum_required(VERSION 3.15)

project(project)

add_executable(project main.c)

add_subdirectory(delHiddenFiles)
add_subdirectory(dstring)

target_link_libraries(project delHiddenFiles)
# 添加链接 dstring 库
target_link_libraries(project dstring)