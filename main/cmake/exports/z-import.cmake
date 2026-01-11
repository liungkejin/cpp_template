# znative-config.cmake

# CMAKE_CURRENT_LIST_FILE
message(STATUS "CMAKE_CURRENT_LIST_FILE: ${CMAKE_CURRENT_LIST_FILE}")
get_filename_component(ZZZ_ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
message(STATUS "ZZZ_ROOT_DIR: ${ZZZ_ROOT_DIR}")

include(${ZZZ_ROOT_DIR}/z-functions.cmake)
set(ZZZ_CMAKE_DIR ${ZZZ_ROOT_DIR}/cmake)
set(ZZZ_INCLUDE_DIR ${ZZZ_ROOT_DIR}/include)
set(ZZZ_LIBS_DIR ${ZZZ_ROOT_DIR}/libs/${ZTARGET_ARCH})

## libname cmake 的文件前缀名称 xxx-arm64-v8a-config.cmake
## 静默查找库, 并设置 ${find_result} 为 TRUE 或 FALSE
## find_path: 查找路径
## libname: 库名称
## find_result: 查找结果, 为 TRUE 或 FALSE
## usage:
## z_find_package_quiet( ${find_path} ${libname} FIND_RESULT)
function(z_find_package_quiet find_path libname find_result)
    set(cfname "${libname}-${ZTARGET_ARCH}")
    set(${cfname}_DIR "${find_path}")
    find_package(${cfname} QUIET)
    if (TARGET ${libname})
        message(STATUS "${cfname} found library: ${libname}")
        set(${find_result} TRUE PARENT_SCOPE)
    else ()
        message(STATUS "${cfname} not found")
        set(${find_result} FALSE PARENT_SCOPE)
    endif ()
    unset(${cfname}_DIR CACHE)
endfunction()

## 查找必要的库
## find_path: 查找路径
## libname: 库名称
function(z_find_package_require find_path libname)
    set(cfname "${libname}-${ZTARGET_ARCH}")
    set(${cfname}_DIR "${find_path}")
    find_package(${cfname} REQUIRED)
    message(STATUS "${cfname} found library: ${libname}")
    unset(${cfname}_DIR CACHE)
endfunction()

## 静默导入静态库
## find_path: 查找路径
## filename: 库文件名
## libname: 库名称
## usage:
## z_import_static_library_quiet(${find_path} ${filename} ${libname} ${deplibs})
function(z_import_static_library_quiet find_path filename libname)
    set(deplibs ${ARGN})
    set(filepath "${find_path}/${ZTARGET_ARCH}/${filename}.a")
    if (WIN32)
        set(filepath "${find_path}/${ZTARGET_ARCH}/${filename}.lib")
        if (NOT EXISTS "${filepath}")
            set(filepath "${find_path}/${ZTARGET_ARCH}/${filename}d.lib")
        endif ()
    endif ()
    if (NOT EXISTS "${filepath}")
        message(WARNING "${libname} not found: ${filepath}")
        return()
    endif ()

    add_library(${libname} STATIC IMPORTED)
    set_target_properties(${libname} PROPERTIES
            IMPORTED_LOCATION "${filepath}"
            INTERFACE_LINK_LIBRARIES "${deplibs}"
    )
    set(ZNATIVE_LIBRARIES ${ZNATIVE_LIBRARIES} ${libname})
    message(STATUS "${libname} found: ${filepath}")
endfunction()

## 导入必要的静态库
## find_path: 查找路径
## filename: 库文件名
## libname: 库名称
## usage:
## z_import_static_library_require(${find_path} ${filename} ${libname} ${deplibs})
function(z_import_static_library_require find_path filename libname)
    z_import_static_library_quiet(${find_path} ${filename} ${libname} ${ARGN})
    if (NOT TARGET ${libname})
        message(FATAL_ERROR "${libname} not found: ${filepath}")
    endif ()
endfunction()

function(z_import_my_package name)
    z_find_package_quiet(${ZZZ_CMAKE_DIR} ${name} FOUND)
    if (NOT FOUND)
        message(FATAL_ERROR "package ${name} not found")
    endif ()

    z_get_target_dep_libs(${name} ZZZ_DEP_LIBRARIES)
    z_get_target_definitions(${name} ZZZ_DEFINITIONS)

    message(STATUS "${name} LIBRARIES: ${ZZZ_DEP_LIBRARIES}")
    message(STATUS "${name} DEFINITIONS: ${ZZZ_DEFINITIONS}")
    # 遍历 ZZZ_DEP_LIBRARIES
    foreach (libname ${ZZZ_DEP_LIBRARIES})
        # 挨个查找
        z_find_package_quiet(FOUND ${libname})
        if (NOT FOUND)
            # 使用 find_package 查找, 如果碰到 OpenGL::GL 这样的应该如何处理？
            # 将 OpenGL::GL 这样的转换为 OpenGL;GL
            set(pkgname ${libname})
            if (libname MATCHES "::")
                string(REPLACE "::" ";" libname ${libname})
                list(GET libname 0 pkgname)
            endif ()
            find_package(${pkgname} QUIET)
            if (TARGET ${libname})
                message(STATUS "found package: ${pkgname}")
            else ()
                message(DEBUG "not found package: ${pkgname}, ${libname}")
            endif ()

            unset(${pkgname}_DIR CACHE)
        endif ()
    endforeach ()
endfunction()