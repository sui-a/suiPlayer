# 1. 规定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.12)

# 2. 定义项目名称、版本号以及使用的语言
project(suiVideoSearchTest
        VERSION 1.0.0 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器特有扩展，确保跨平台
#set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra -Wpedantic -O3) # Linux/Mac 环境，开启O3优化

#创建文件
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)

#收集源文件头文件
file(GLOB CUR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../../../data/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../util/*.hpp")
file(GLOB CUR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../../data/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../util/*.cpp")
file(GLOB GOAL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

#编译odb与proto
#收集所有proto文件路径
file(GLOB_RECURSE PROTO_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../proto/*.proto")
#寻找protoc可执行文件
find_program(PROTOC_EXECUTABLE protoc REQUIRED)
execute_process(
    COMMAND ${PROTOC_EXECUTABLE}
            --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../../../proto
            --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/src
            ${PROTO_FILE_PATH}
    RESULT_VARIABLE protoc_ret         # 将执行结果代码（0代表成功，非0代表失败）赋值给 protoc_ret
    OUTPUT_VARIABLE protoc_stdout      # 捕获标准输出
    ERROR_VARIABLE protoc_stderr       # 捕获错误信息
)
if(protoc_ret)
    message(FATAL_ERROR  "protoc失败: ${protoc_stderr}")
endif()

#开始编译odb
set(ODB_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../data/data.hpp"
)
#设置odb输出目录
set(ODB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
#循环编译文件
find_program(ODB_EXECUTABLE odb REQUIRED)
#/usr/bin/odb --database mysql --generate-query --std c++11 -I./ ./data.hpp
foreach(odb_header ${ODB_HEADERS})
    #odb -d mysql --generate-query --generate-schema --schema-format embedded YourEntityClass.hxx
    execute_process(
        COMMAND ${ODB_EXECUTABLE}
                --database mysql --generate-query  --generate-schema --std c++11  --schema-format sql --schema-format embedded
                -I${CMAKE_CURRENT_BINARY_DIR}/src ${odb_header}
                -I${CMAKE_CURRENT_SOURCE_DIR}/../../../util ${odb_header}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src # 设置工作目录为src
        RESULT_VARIABLE protoc_ret         # 将执行结果代码（0代表成功，非0代表失败）赋值给 protoc_ret
        OUTPUT_VARIABLE protoc_stdout      # 捕获标准输出
        ERROR_VARIABLE protoc_stderr       # 捕获错误信息
    )
    if(protoc_ret)
        message(FATAL_ERROR  "odb编译${odb_header}失败: ${protoc_stderr}")
    endif()
endforeach()

#收集源文件
file(GLOB_RECURSE MY_GEN_ODB_CPP 
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/src/*.hxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../util/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../util/*.hpp"
)

add_executable(${PROJECT_NAME} ${CUR_SOURCES} ${MY_GEN_ODB_CPP} ${GOAL_FILES})

#寻找依赖
find_package(suiScaffold REQUIRED)

#添加依赖库
set(libs jsoncpp protobuf spdlog
    ev amqpcpp pthread brpc leveldb dl
    ssl crypto gflags cpr elasticlient
    cpprest etcd-cpp-api fdfsclient
    fastcommon avcodec avformat gtest
    curl odb-mysql odb odb-boost
    hiredis fmt redis++
)

#添加头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../../../data ${CMAKE_CURRENT_SOURCE_DIR}/../../../util)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  suiScaffold::suiScaffold
  ${libs}
)