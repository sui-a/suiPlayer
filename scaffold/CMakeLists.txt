#设置cmake最小版本
cmake_minimum_required(VERSION 3.10)
#设置项目名称
project(suiScaffold VERSION 1.0)

#添加生成库目标
file(GLOB_RECURSE suiBaseFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

#定义库
add_library(${PROJECT_NAME} STATIC ${suiBaseFiles})   # 或 SHARED
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
#设置编译特性
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
#设置生产库的属性，版本信息
set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION} #完整版本号
    SOVERSION ${PROJECT_VERSION_MAJOR} #主版本号
)

#依赖检测
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake) #声明依赖子模块位置
include(FindDepends)
find_my_depends()
add_my_depends()

#开始安装头文件
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

#安装库文件
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets # 声明目标名称
    ARCHIVE DESTINATION lib #静态库安装路径
    INCLUDES DESTINATION include/${PROJECT_NAME} #头文件安装路径
)

#安装子模块到特定位置
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/
    DESTINATION lib/cmake/${PROJECT_NAME}/modules
    FILES_MATCHING PATTERN "Find*cmake"
)

#设置导出目标信息
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
)

#处理Config.cmake.in模板
# 配置并安装版本文件
include(CMakePackageConfigHelpers)
# 处理 Config.cmake.in 模板，生成最终的配置文件，解析 @PACKAGE_INIT@ 变量
configure_package_config_file(
  cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

#设置版本配置文件
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# 13. 安装配置文件
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION lib/cmake/${PROJECT_NAME}
)

