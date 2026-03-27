cmake_minimum_required(VERSION 3.18)
if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()

##
## cpp 模版工程，注意修改 cpp_template 为你自己的工程名称
##
project(cpp_template)

set(CMAKE_CXX_STANDARD 17)

# 主目标名称, 注意修改为你自己的工程名称
set(MAIN_TARGET ${PROJECT_NAME})
#add_library(${MAIN_TARGET} SHARED)
add_library(${MAIN_TARGET} STATIC)

message(STATUS "====== Target: ${MAIN_TARGET} =====")

set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main)
# 将资源文件嵌入到二进制文件中
include(${MAIN_DIR}/cmake/CMakeRC.cmake)
# 自己定义的一些工具函数
include(${MAIN_DIR}/cmake/exports/z-functions.cmake)

# 公共第三方库路径、源文件、头文件路径
set(PUBLIC_DIR ${MAIN_DIR}/public)
set(PUBLIC_LIBS_DIR ${PUBLIC_DIR}/libs)
set(PUBLIC_SRC_DIR ${PUBLIC_DIR}/src)
set(PUBLIC_INCLUDE_DIR ${PUBLIC_DIR}/include)
# 平台相关第三方库、源文件、头文件路径
set(PLATFORM_DIR ${MAIN_DIR}/platforms/${ZPLATFORM})
set(PLATFORM_LIBS_DIR ${PLATFORM_DIR}/libs)
set(PLATFORM_SRC_DIR ${PLATFORM_DIR}/src)
set(PLATFORM_INCLUDE_DIR ${PLATFORM_DIR}/include)

# 安装目录: install/${ZPLATFORM}/${CMAKE_BUILD_TYPE}
# 结构:
# include/
# libs/${ZTARGET_ARCH}/
# cmake/
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/${ZPLATFORM}/${CMAKE_BUILD_TYPE})
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/libs/${ZTARGET_ARCH})

# 引入并安装所有的头文件
z_target_include_public_build_interface_default(${MAIN_TARGET}
        ${PUBLIC_INCLUDE_DIR}/
        ${PLATFORM_INCLUDE_DIR}/
)
# 指定安装后头文件的路径
target_include_directories(${MAIN_TARGET} PUBLIC $<INSTALL_INTERFACE:include/>)


include(${PLATFORM_DIR}/platform.cmake)
include(${PUBLIC_DIR}/public.cmake)

z_install_target(${MAIN_TARGET})
z_install_files_default(
        ${MAIN_DIR}/cmake/exports/z-functions.cmake
        ${MAIN_DIR}/cmake/CMakeRC.cmake
)

configure_file("${MAIN_DIR}/cmake/exports/z-import.cmake.in" "${CMAKE_BINARY_DIR}/z-import.cmake" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/z-import.cmake" DESTINATION .)
# 配置并安装 README.md
configure_file("${MAIN_DIR}/cmake/exports/README.md.in" "${CMAKE_BINARY_DIR}/README.md" @ONLY)
# 安装生成后的文件
install(FILES "${CMAKE_BINARY_DIR}/README.md" DESTINATION .)

# sample 目标
if (ZDESKTOP)
    add_subdirectory(samples/local)
endif ()
