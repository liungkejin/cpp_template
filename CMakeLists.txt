cmake_minimum_required(VERSION 3.30)
##
## cpp 模版工程，注意修改 cpp_template 为你自己的工程名称
##
project(cpp_template)

#include(FetchContent)
#FetchContent_Declare(
#        znative
#        GIT_REPOSITORY http://192.168.3.71/benqu_alg/znative.git
#        GIT_TAG main
#)
#
#FetchContent_MakeAvailable(znative)

set(CMAKE_CXX_STANDARD 17)

# 主目标名称, 注意修改为你自己的工程名称
set(MAIN_TARGET ${PROJECT_NAME})
message(STATUS "====== Target: ${MAIN_TARGET} =====")

set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main)
# 将资源文件嵌入到二进制文件中
include(${MAIN_DIR}/cmake/CMakeRC.cmake)
include(${MAIN_DIR}/cmake/exports/znative-functions.cmake)

# 公共第三方库路径
set(COMMON_LIBS_PATH ${MAIN_DIR}/libs/common)
set(COMMON_SRC_PATH ${MAIN_DIR}/src/common)
# 平台相关第三方库路径
set(PLATFORM_LIBS_PATH ${MAIN_DIR}/libs/${ZPLATFORM})
set(PLATFORM_SRC_PATH ${MAIN_DIR}/src/${ZPLATFORM})
# 导出头文件路径
set(EXPORT_INCLUDES ${MAIN_DIR}/include/)

# 安装目录: install/${ZPLATFORM}/${CMAKE_BUILD_TYPE}
# 结构:
# include/
# libs/${ZTARGET_ARCH}/
# cmake/
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/${ZPLATFORM}/${CMAKE_BUILD_TYPE})
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/libs/${ZTARGET_ARCH})

add_library(${MAIN_TARGET} SHARED)
#add_library(${MAIN_TARGET} STATIC)

target_include_directories(${MAIN_TARGET} PRIVATE
        ${PLATFORM_SRC_PATH}
        ${COMMON_SRC_PATH}
)
target_include_directories(${MAIN_TARGET} PRIVATE
        ${EXPORT_INCLUDES}
)
target_sources(${MAIN_TARGET} PRIVATE
        ${PLATFORM_SRC_PATH}/Platform.cpp
)

if (ZANDROID)
    include(${MAIN_DIR}/cmake/platforms/Android.cmake)
elseif (ZOHOS)
    include(${MAIN_DIR}/cmake/platforms/HarmonyOS.cmake)
elseif (ZIOS)
    include(${MAIN_DIR}/cmake/platforms/iOS.cmake)
else ()
    include(${MAIN_DIR}/cmake/platforms/Desktop.cmake)
endif ()

include(${MAIN_DIR}/cmake/Common.cmake)
z_install_target(${MAIN_TARGET})


# sample 目标
add_executable(${MAIN_TARGET}-sample samples/src/main.cpp)
target_link_libraries(${MAIN_TARGET}-sample PRIVATE ${MAIN_TARGET})
