# delHiddenFiles/CMakeLists.txt
add_library(delHiddenFiles delHiddenFiles.c)

# 指定delHiddenFiles的头文件目录
target_include_directories(delHiddenFiles PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR} 
)

# 指定依赖的dstring头文件目录
target_include_directories(delHiddenFiles PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/../dstring
)