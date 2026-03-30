cmake_minimum_required(VERSION 3.10)

project(local-sample)

set(LOCAL_SAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SAMPLE_LIBS_DIR ${LOCAL_SAMPLE_DIR}/libs)
set(SAMPLE_SRC_DIR ${LOCAL_SAMPLE_DIR}/src)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_LIBRARY_TYPE STATIC)
add_subdirectory(${SAMPLE_LIBS_DIR}/glfw-3.4)

set(IMGUI_DIR ${SAMPLE_LIBS_DIR}/imgui-1.92.5)
include_directories(
        ${SAMPLE_LIBS_DIR}/glfw-3.4/include
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${SAMPLE_INCLUDES}
)

set(IMGUI_SRC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(SAMPLE_SOURCES
        ${SAMPLE_SOURCES}
        ${SAMPLE_SRC_DIR}/main.cpp
        ${SAMPLE_SRC_DIR}/MainApp.cpp
        ${SAMPLE_SRC_DIR}/ThorvgTestWindow.cpp
)

set(TARGET_NAME local-sample)
add_executable(${TARGET_NAME}
        ${IMGUI_SRC}
        ${SAMPLE_SOURCES}
)

# 嵌入资源到可执行文件
z_embed_res_to_target_private(
        ${TARGET_NAME}
        local
        ${LOCAL_SAMPLE_DIR}
        assets/
)

z_set_msvc_runtime(${TARGET_NAME} ON)

target_include_directories(${TARGET_NAME} PRIVATE
        ${MAIN_DIR}/src/
)
# 显式链接数学库，在编译静态库的时候是必须的
set(SAMPLE_LIBS glfw)

if (WIN32)
    set(SAMPLE_LIBS ${SAMPLE_LIBS} glew_s msvcrtd.lib libcmt.lib)
    set(SAMPLE_INCLUDES ${SAMPLE_INCLUDES} ${GLEW_DIR}/include)
else ()
    find_package(OpenGL REQUIRED)
    set(SAMPLE_LIBS ${SAMPLE_LIBS} OpenGL::GL)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DGL_SILENCE_DEPRECATION)
endif ()

set(thorvgpath ${SAMPLE_LIBS_DIR}/thorvg/build/install/${ZPLATFORM}/${ZTARGET_ARCH})
if(NOT EXISTS ${thorvgpath}/lib/libthorvg-1.a)
    message(STATUS "Building thorvg...")
    execute_process(
            COMMAND ${SAMPLE_LIBS_DIR}/thorvg/build.sh local static release
            WORKING_DIRECTORY ${SAMPLE_LIBS_DIR}/thorvg
            OUTPUT_VARIABLE OUTPUT
            ERROR_VARIABLE ERROR
    )
endif ()
z_import_static_library(thorvg ${thorvgpath}/lib/libthorvg-1.a ${thorvgpath}/include "")
set(SAMPLE_LIBS ${SAMPLE_LIBS} thorvg)

target_link_libraries(${TARGET_NAME} PRIVATE
        ${MAIN_TARGET}
        ${SAMPLE_LIBS}
)
