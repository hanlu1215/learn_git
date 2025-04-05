cmake_minimum_required(VERSION 3.10)
project(Demo)
# set(CMAKE_CXX_STANDARD 11)
# # 手动查找xlnt库
# find_library(xlnt xlnt ${PROJECT_SOURCE_DIR}/vcpkg/packages/xlnt_x64-mingw-static/lib)
# include_directories(${PROJECT_SOURCE_DIR}/vcpkg/packages/xlnt_x64-mingw-static/include)

set(CMAKE_EXE_LINKER_FLAGS "-static") # GCC/Clang环境‌:ml-citation{ref="5,8" data="citationList"}
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static") # 兼容Windows/MinGW‌:ml-citation{ref="5" data="citationList"}
# 设置构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


# 将 vcpkg 安装目录添加到 CMAKE_PREFIX_PATH 中
# set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-mingw-static" CACHE STRING "Prefix paths for vcpkg installed packages" FORCE)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-mingw-static")
set(Protobuf_PROTOC_EXECUTABLE "${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-mingw-static/tools/protobuf/protoc.exe")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)  # 动态库（Windows .dll）和可执行文件‌:ml-citation{ref="2,4" data="citationList"}
set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# find_package(Xlnt CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(LIBS
    ${OpenCV_LIBS})

add_executable(main ${PROJECT_SOURCE_DIR}/src/merge_video.cpp)
# 链接库
target_link_libraries(main ${LIBS})
