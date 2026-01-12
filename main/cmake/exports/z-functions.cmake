##
# 注意 function 的 parent scope 只对上一层有用，不能多层
##

# 获取系统信息
# out_name: 输出的系统名称 [ android | harmony | windows | linux | macos ]
# out_arch: 输出的系统架构 [ x86 | x64 | arm | arm64 | universal ]
# 平台判断可以使用 ZANDROID | ZOHOS | ZIOS | ZLINUX | ZMACOS | ZWINDOWS | ZDESKTOP
function(z_get_sys_info out_name out_arch)
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        set(${out_name} "android" PARENT_SCOPE)
        set(${out_arch} ${CMAKE_ANDROID_ARCH_ABI} PARENT_SCOPE)
        set(ZANDROID ON PARENT_SCOPE)
    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "OHOS")
        set(${out_name} "harmony" PARENT_SCOPE)
        set(${out_arch} ${OHOS_ARCH} PARENT_SCOPE)
        set(ZOHOS ON PARENT_SCOPE)
        set(ZHARMONY ON PARENT_SCOPE)
    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
        set(${out_name} "ios" PARENT_SCOPE)
        set(${out_arch} "arm64" PARENT_SCOPE)
        set(ZIOS ON PARENT_SCOPE)
    else ()
        set(target_arch ${CMAKE_SYSTEM_PROCESSOR})
        set(ZDESKTOP ON PARENT_SCOPE)
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
            set(${out_name} "linux" PARENT_SCOPE)
            set(ZLINUX ON PARENT_SCOPE)
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
            set(${out_name} "macos" PARENT_SCOPE)
            set(ZMACOS ON PARENT_SCOPE)
            # 这个变量是定义在 CMakePresets.json 中的
            if (CMAKE_OSX_ARCHITECTURES)
                set(target_arch ${CMAKE_OSX_ARCHITECTURES})
            endif ()
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
            set(${out_name} "windows" PARENT_SCOPE)
            set(ZWINDOWS ON PARENT_SCOPE)
        else ()
            message(FATAL_ERROR "Unknown system: ${CMAKE_HOST_SYSTEM_NAME}")
        endif()

        if ("${target_arch}" MATCHES "amd" OR
                "${target_arch}" MATCHES "AMD" OR
                "${target_arch}" MATCHES "x86")
            # 如果是 macos 并且是包含 x86_64 和 arm64 架构，那么是 universal 架构
            if (CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin AND "${target_arch}" MATCHES "arm64")
                set(${out_arch} "universal" PARENT_SCOPE)
            else ()
                if ("${target_arch}" MATCHES "64")
                    set(${out_arch} "x64" PARENT_SCOPE)
                else ()
                    set(${out_arch} "x86" PARENT_SCOPE)
                endif ()
            endif ()
        elseif ("${target_arch}" MATCHES "arm")
            if ("${target_arch}" MATCHES "64")
                set(${out_arch} "arm64" PARENT_SCOPE)
            else ()
                set(${out_arch} "arm" PARENT_SCOPE)
            endif ()
        else ()
            message(FATAL_ERROR "Unknown arch: ${target_arch}")
            set(${out_arch} "${target_arch}" PARENT_SCOPE)
        endif ()
    endif ()
endfunction()

# 获取系统信息
z_get_sys_info(ZPLATFORM ZTARGET_ARCH)
message(STATUS "====== ZPLATFORM: ${ZPLATFORM} ======")
message(STATUS "====== ZTARGET_ARCH: ${ZTARGET_ARCH} ======")

# 安装任意文件到 ${install_dir} 目录下
# usage:
# z_install_files(${CMAKE_INSTALL_INCLUDEDIR} "a/*" "b/c.txt" "dir/")
# 如果是文件夹，使用 install(DIRECTORY) 安装, 注意如果 file 是以 / 结尾的，会安装 file 目录下的所有文件，而不是 file 目录本身
function(z_install_files install_dir)
    foreach (file ${ARGN})
        if (file MATCHES "\\*")
            # 如果 file 包含 * 字符，使用 FILES_MATCHING PATTERN
            get_filename_component(dir ${file} DIRECTORY)
            get_filename_component(pattern ${file} NAME)
            if (IS_DIRECTORY ${dir})
                install(DIRECTORY ${dir}/
                        DESTINATION ${install_dir}
                        FILES_MATCHING PATTERN ${pattern})
            else ()
                message(WARNING "Directory ${dir} does not exist. Skipping installation. ${file}")
            endif ()
        elseif (IS_DIRECTORY ${file})
            # 如果是文件夹，使用 install(DIRECTORY) 安装, 注意如果 file 是以 / 结尾的，会安装 file 目录下的所有文件，而不是 file 目录本身
            install(DIRECTORY ${file} DESTINATION ${install_dir})
        elseif (EXISTS ${file} OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
            # 如果是文件，使用 install(FILES) 安装
            install(FILES ${file} DESTINATION ${install_dir})
        else ()
            message(WARNING "File ${file} does not exist. Skipping installation.")
        endif ()
    endforeach ()
endfunction()

function(z_install_files_default)
    z_install_files(${CMAKE_INSTALL_PREFIX} ${ARGN})
endfunction()

## 安装头文件，支持目录和文件
## usage：
## z_install_single_include(<dst dir> <src path>)
function(z_install_single_include dst_dir src_path)
    if (IS_DIRECTORY ${src_path})
        install(DIRECTORY ${src_path} DESTINATION ${dst_dir}
                FILES_MATCHING PATTERN "*.h")
        install(DIRECTORY ${src_path} DESTINATION ${dst_dir}
                FILES_MATCHING PATTERN "*.hpp")
    elseif (EXISTS ${src_path} OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src_path}")
        # 如果是文件，使用 install(FILES) 安装
        install(FILES ${src_path} DESTINATION ${dst_dir})
    else ()
        message(WARNING "${src_path} does not exist. Skipping installation.")
    endif ()
endfunction()

## 安装头文件, 支持多个路径
## usage：
## z_install_includes <dst dir> "/a/b/d->e/f"
## z_install_includes(${CMAKE_INSTALL_INCLUDEDIR} "/a/b/c/d->e/f"
## "/a/b/d" 是原路径
## "e/f" 是目标的相对路径即 install_dir/e/f 如果没有指定目标路径，默认安装到 install_dir
function(z_install_includes install_dir)
    foreach (file ${ARGN})
        # 判断 src_path 是否已经包含 $<BUILD_INTERFACE
        # 如果包含 $<BUILD_INTERFACE 则不添加
        if (file MATCHES "\\$\\<BUILD_INTERFACE:")
            # 将 $<BUILD_INTERFACE:xxx> 中的 xxx 提取出来
            string(REPLACE "$<BUILD_INTERFACE:" "" file ${file})
        endif ()
        string(REPLACE ">" "" file ${file})

        string(REPLACE "->" ";" file ${file})
        list(LENGTH file file_len)
        set(src_path ${file})
        set(dst_dir ${install_dir})
        if (${file_len} EQUAL 2)
            list(GET file 0 src_path)
            list(GET file 1 dst_dir)
            set(dst_dir ${install_dir}/${dst_dir})

            # 如果 src_path 包含很多个路径，每个路径都需要安装
            if (src_path MATCHES ";")
                foreach (src ${src_path})
                    z_install_single_include(${dst_dir} ${src})
                endforeach ()
            else ()
                z_install_single_include(${dst_dir} ${src_path})
            endif ()
        else ()
            # 如果 src_path 包含很多个路径，每个路径都需要安装
            if (src_path MATCHES ";")
                foreach (src ${src_path})
                    z_install_single_include(${dst_dir} ${src})
                endforeach ()
            else ()
                z_install_single_include(${dst_dir} ${src_path})
            endif ()
        endif ()

    endforeach ()
endfunction()

## 默认安装到 CMAKE_INSTALL_INCLUDEDIR 目录
## usage:
## z_install_includes_default("a/b/c.h" "a/b/d.h->e/f.h")
#"${COMMON_SRC_PATH}/../ZNative.h->znative/"
#"${COMMON_SRC_PATH}/Common.h->znative/common"
#"${COMMON_SRC_PATH}/AppContext.h->znative/common"
#"${COMMON_SRC_PATH}/base->znative/common"
#"${COMMON_SRC_PATH}/fsys->znative/common"
#"${COMMON_SRC_PATH}/thread->znative/common"
#"${COMMON_SRC_PATH}/utils->znative/common"
#"${COMMON_SRC_PATH}/media->znative/common"
function(z_install_includes_default)
    z_install_includes(${CMAKE_INSTALL_INCLUDEDIR} ${ARGN})
endfunction()

## target public引用头文件，并安装到 install_dir (public应用意味着在安装之后也可以被其他应用引用)
## 这个函数虽然也支持 $<BUILD_INTERFACE:${include_dir}> 方式引用头文件，
## 但是对于 ${include_dir} 中存在多个路径的情况，则会在安装后的 INTERFACE_INCLUDE_DIRECTORIES 中包含一个 CMAKE_CURRENT_SOURCE_DIR 路径
## usage:
## z_target_include_public(<target_name> <install_dir> <include_dir1> <include_dir2> ...)
function(z_target_include_public target_name install_dir)
    z_install_includes(${install_dir} ${ARGN})
    target_include_directories(${target_name} PUBLIC
            ${ARGN}
    )
    message(STATUS "z_target_include_public: ${target_name} ${ARGN} -> ${install_dir}")
endfunction()

function(z_target_include_public_default target_name)
    z_target_include_public(${target_name} ${CMAKE_INSTALL_INCLUDEDIR} ${ARGN})
endfunction()

## target 使用 $<BUILD_INTERFACE:${include_dir}> 方式引用头文件，
## 并安装到 install_dir (interface应用意味着在安装之后不会被其他应用引用)
## usage:
## z_target_include_public_build_interface(<target_name> <install_dir> <include_dir1> <include_dir2> ...)
function(z_target_include_public_build_interface target_name install_dir)
    foreach (include_dir ${ARGN})
        z_install_includes(${install_dir} ${include_dir})
        target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${include_dir}>)
    endforeach ()
endfunction()

function(z_target_include_public_build_interface_default target_name)
    z_target_include_public_build_interface(${target_name} "${CMAKE_INSTALL_INCLUDEDIR}" ${ARGN})
endfunction()

# import static library
function(z_import_static_library target_name lib_path libfilename deplibs)
    #    message(STATUS "import static library: ${target_name}  ${lib_path}  ${libfilename}")
    #    message(STATUS "depends library: ${deplibs}")
    set(lib_include "${lib_path}/include/")
    set(lib_static "${lib_path}/libs/${ZTARGET_ARCH}/${libfilename}")
    if (NOT EXISTS ${lib_static})
        message(FATAL_ERROR "static library ${lib_static} not found")
    endif ()
    add_library(${target_name} STATIC IMPORTED)
    set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${lib_static}"
            INTERFACE_INCLUDE_DIRECTORIES "${lib_include}"
            INTERFACE_LINK_LIBRARIES "${deplibs}"
    )
    #    message(STATUS "import static library: ${target_name} | ${lib_static} | ${lib_include} | ${deplibs}")
endfunction()

# import shared library
function(z_import_shared_library target_name lib_path libfilename deplibs)
    #    message(STATUS "import shared library: ${target_name}  ${lib_path}  ${libfilename}")
    #    message(STATUS "depends library: ${deplibs}")
    set(lib_include "${lib_path}/include/")
    set(lib_shared "${lib_path}/libs/${ZTARGET_ARCH}/${libfilename}")
    if (NOT EXISTS ${lib_shared})
        message(FATAL_ERROR "shared library ${lib_shared} not found")
    endif ()
    add_library(${target_name} SHARED IMPORTED)
    set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${lib_shared}"
            INTERFACE_INCLUDE_DIRECTORIES "${lib_include}"
            INTERFACE_LINK_LIBRARIES "${deplibs}"
    )
    #    message(STATUS "import shared library: ${target_name} | ${lib_shared} | ${lib_include} | ${deplibs}")
endfunction()

# 设置 MSVC 的运行时 MT / MD
## 可以使用命令查看库的运行时 dumpbin /DIRECTIVES znative-static.lib | findstr "Runtime"
## target_name: 目标名称
## mt_runtime: 是否使用 MT 运行时 [ON/OFF]
function(z_set_msvc_runtime target_name mt_runtime)
    if (MSVC)
        if (POLICY CMP0091)
            cmake_policy(SET CMP0091 NEW)
        endif ()
        if (${mt_runtime})
            # $<CONFIG:Debug> 是条件表达式，当 CMAKE_BUILD_TYPE 为 Debug 时，条件为真
            # $<$<CONFIG:Debug>:Debug> 表示当 CMAKE_BUILD_TYPE 为 Debug 时，条件为真，表达式的值为 Debug, 否则返回空字符串
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" PARENT_SCOPE)
            if (NOT "${target_name}" STREQUAL "")
                set_target_properties(${target_name} PROPERTIES
                        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
                )
            endif ()

            foreach (flag_var
                    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                if (${flag_var} MATCHES "/MD")
                    string(REGEX REPLACE "/MD" "/MT" out_var "${${flag_var}}")
                    set(${flag_var} "${out_var}" PARENT_SCOPE)
                endif ()
                if (${flag_var} MATCHES "/MDd")
                    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
                        string(REGEX REPLACE "/MDd" "/MTd" out_var "${${flag_var}}")
                    else ()
                        string(REGEX REPLACE "/MDd" "/MT" out_var "${${flag_var}}")
                    endif ()
                    set(${flag_var} "${out_var}" PARENT_SCOPE)
                endif ()
            endforeach (flag_var)
        else ()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" PARENT_SCOPE)
            if (NOT "${target_name}" STREQUAL "")
                set_target_properties(${target_name} PROPERTIES
                        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
                )
            endif ()

            foreach (flag_var
                    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                if (${flag_var} MATCHES "/MT")
                    string(REGEX REPLACE "/MT" "/MD" out_var "${${flag_var}}")
                    set(${flag_var} "${out_var}" PARENT_SCOPE)
                endif ()
                if (${flag_var} MATCHES "/MTd")
                    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
                        string(REGEX REPLACE "/MTd" "/MDd" out_var "${${flag_var}}")
                    else ()
                        string(REGEX REPLACE "/MTd" "/MD" out_var "${${flag_var}}")
                    endif ()
                    set(${flag_var} "${out_var}" PARENT_SCOPE)
                endif ()
            endforeach (flag_var)
        endif ()
    endif ()
endfunction()

# 封装 set(OpenCV_DIR "...")  find_package(OpenCV 4.3.0)
# 直接使用 z_find_package("..." OpenCV 4.3.0)
# name: 包的名称
# path: package 的路径
function(z_find_package name path)
    if (NOT path STREQUAL "")
        if (NOT EXISTS ${path})
            message(FATAL_ERROR "path ${path} not exists, find package ${name} failed.")
        endif ()
        set(${name}_DIR "${path}")
    endif ()
    find_package(${name} ${ARGN})
endfunction()

# 获取一个 target 所依赖的头文件目录
function(z_get_target_dep_includes target out_inc)
    get_target_property(lib_include ${target} INTERFACE_INCLUDE_DIRECTORIES)
    set(${out_inc} ${lib_include} PARENT_SCOPE)
endfunction()

# 获取一个 target 所依赖的库
function(z_get_target_dep_libs target out_libs)
    get_target_property(lib_deplibs ${target} INTERFACE_LINK_LIBRARIES)
    set(${out_libs} ${lib_deplibs} PARENT_SCOPE)
endfunction()

# 获取一个 target 所依赖的编译定义
function(z_get_target_definitions target out_defs)
    get_target_property(lib_defs ${target} INTERFACE_COMPILE_DEFINITIONS)
    set(${out_defs} ${lib_defs} PARENT_SCOPE)
endfunction()

# 使用 CMakeRC.cmake 将资源文件嵌入到 ${target}-rc 中，并链接到 ${target}
# @param lib_name: 资源文件库名称
# @param namespace: 资源文件的命名空间
# @param whence: 资源文件的根路径, 比如一个文件是：${CMAKE_CURRENT_SOURCE_DIR}/res/fonts/fa-solid-900.ttf,
# 则 whence 为 ${CMAKE_CURRENT_SOURCE_DIR}, 资源文件的路径为 res/fonts/fa-solid-900.ttf
# @param res_files: 资源文件的路径列表 可以是一个文件夹, 也可以是一个文件，如果是文件夹
# 则递归遍历文件夹中的所有文件
# usage:
# z_embed_resources(znative assets ${CMAKE_CURRENT_SOURCE_DIR}/res fonts/fa-solid-900.ttf images)
# 代码中的使用:
# #include <cmrc/cmrc.hpp>
# CMRC_DECLARE(${namespace});
# auto file = CMRC_FS(${namespace}).open("fonts/fa-solid-900.ttf");
# auto data = file.begin();
# auto size = file.size();
function(z_embed_resources lib_name namespace whence)
    if (NOT TARGET ${lib_name})
        cmrc_add_resource_library(${lib_name} NAMESPACE ${namespace} WHENCE "${whence}")
    endif ()
    # 遍历 ARGN, 如果是目录，则遍历目录中的所有文件，递归调用 z_embed_res_files
    foreach (res_file ${ARGN})
        # 判断是否存在
        if (NOT EXISTS ${res_file})
            set(res_file "${whence}/${res_file}")
        endif ()
        # 如果 res_file 带有重复的 //, 则去掉重复的 //
        string(REGEX REPLACE "//+" "/" res_file ${res_file})
        if (NOT EXISTS ${res_file})
            message(WARNING "CMRC embed resource failed: ${res_file} not exists.")
        elseif (IS_DIRECTORY ${res_file})
            # list all files in ${res_file}
            file(GLOB_RECURSE res_files ${res_file}/*)
#            message("CMRC embed directory: ${res_files}")
            z_embed_resources(${lib_name} ${namespace} "${whence}" ${res_files})
        else ()
            # 直接添加文件
            message("CMRC embed file to ${lib_name}: ${res_file}")
            cmrc_add_resources(${lib_name} WHENCE "${whence}" ${res_file})
        endif ()
    endforeach ()
endfunction()

function(z_embed_install_includes dstdir)
    z_install_includes(${dstdir} ${CMAKE_BINARY_DIR}/_cmrc/include/)
endfunction()

# 将资源文件嵌入到 ${target}-rc 中，并链接到 ${target}
function(z_embed_res_to_target target namespace whence)
    set(lib_name ${target}-rc)
    set(first_create OFF)
    if (NOT TARGET ${lib_name})
        set(first_create ON)
    endif ()
    z_embed_resources(${lib_name} ${namespace} "${whence}" ${ARGN})
    if (${first_create})
        target_link_libraries(${target} PUBLIC ${lib_name})
        z_install_target(${lib_name})
        z_install_includes(${CMAKE_INSTALL_PREFIX}/include ${CMAKE_BINARY_DIR}/_cmrc/include/)
    endif ()
endfunction()

# install target with namespace
# target_name: 目标名称
# namespace: 命名空间 没有的话可以留空
# target 会安装在 libs/${ZTARGET_ARCH}/${target_name}
# cmake 会安装在 cmake/${target_name}-${ZTARGET_ARCH}-config.cmake
function(z_install_target_with_namespace target_name namespace)
    set(EXPORT_CONFIG ${target_name}-${ZTARGET_ARCH}-config)
    install(TARGETS ${target_name}
            EXPORT ${EXPORT_CONFIG}
            DESTINATION libs/${ZTARGET_ARCH}
    )
    if (DEFINED namespace AND NOT "${namespace}" STREQUAL "")
        install(EXPORT ${EXPORT_CONFIG}
                FILE ${EXPORT_CONFIG}.cmake
                DESTINATION cmake
                NAMESPACE ${namespace}::
        )
    else ()
        install(EXPORT ${EXPORT_CONFIG}
                FILE ${EXPORT_CONFIG}.cmake
                DESTINATION cmake
        )
    endif ()

    message(STATUS "z-install library: ${namespace} ${target_name} ${EXPORT_CONFIG}")
endfunction()

## install target without namespace
function(z_install_target target_name)
    z_install_target_with_namespace(${target_name} "")
endfunction()
