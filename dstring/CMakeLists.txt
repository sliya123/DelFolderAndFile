# dstring/CMakeLists.txt
add_library(dstring dstring.c) 

# 指定dstring的头文件目录
target_include_directories(dstring PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR} 
)