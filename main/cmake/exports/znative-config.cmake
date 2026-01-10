# znative-config.cmake

# CMAKE_CURRENT_LIST_FILE
message(STATUS "CMAKE_CURRENT_LIST_FILE: ${CMAKE_CURRENT_LIST_FILE}")
get_filename_component(CONFIG_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
message(STATUS "ZCONFIG_DIR: ${CONFIG_DIR}")
message(STATUS "MAIN_DIR: ${MAIN_DIR}")

include(${CONFIG_DIR}/znative-functions.cmake)

set(ZSYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(ZSYSTEM_ARCH ${CMAKE_SYSTEM_PROCESSOR})
z_get_sys_info(ZSYSTEM_NAME ZSYSTEM_ARCH)

message(STATUS "ZSYSTEM_ARCH: ${ZSYSTEM_ARCH}")

set(ZNATIVE_INCLUDES
        "${CONFIG_DIR}/include/znative"
        "${CONFIG_DIR}/include/"
)

## libname cmake 的文件前缀名称 xxx-arm64-v8a-config.cmake
function(z_find_package_quiet find_result libname)
    set(cfname "${libname}-${ZSYSTEM_ARCH}")
    set(${cfname}_DIR "${CONFIG_DIR}/cmake")
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

function(z_find_package_require libname)
    set(cfname "${libname}-${ZSYSTEM_ARCH}")
    set(${cfname}_DIR "${CONFIG_DIR}/cmake")
    find_package(${cfname} REQUIRED)
    message(STATUS "${cfname} found library: ${libname}")
    set(ZNATIVE_LIBRARIES ${ZNATIVE_LIBRARIES} ${libname} PARENT_SCOPE)
    unset(${cfname}_DIR CACHE)
endfunction()

function(z_find_static_library_quiet filename libname)
    set(deplibs ${ARGN})
    set(filepath "${CONFIG_DIR}/libs/${ZSYSTEM_ARCH}/${filename}.a")
    if (WIN32)
        set(filepath "${CONFIG_DIR}/libs/${ZSYSTEM_ARCH}/${filename}.lib")
        if (NOT EXISTS "${filepath}")
            set(filepath "${CONFIG_DIR}/libs/${ZSYSTEM_ARCH}/${filename}d.lib")
        endif ()
    endif ()
    if (NOT EXISTS "${filepath}")
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

function(z_find_static_library_require libname filename deplibs)
    set(filepath "${CONFIG_DIR}/libs/${ZSYSTEM_ARCH}/${filename}.a")
    if (WIN32)
        set(filepath "${CONFIG_DIR}/libs/${ZSYSTEM_ARCH}/${filename}.lib")
    endif ()
    if (NOT EXISTS "${filepath}")
        message(FATAL_ERROR "File ${filepath} does not exist. failed to find ${libname}")
        return()
    endif ()

    add_library(${libname} STATIC IMPORTED)
    set_target_properties(${libname} PROPERTIES
            IMPORTED_LOCATION "${filepath}"
            INTERFACE_LINK_LIBRARIES "${deplibs}"
    )
    message(STATUS "${libname} found: ${filepath}")
endfunction()

z_find_package_quiet(FOUND znative-shared znative-shared)
if (FOUND)
    set(ZNATIVE_LIB znative-shared)
    target_include_directories(znative-shared INTERFACE ${ZNATIVE_INCLUDES})
    message(STATUS "ZNATIVE_LIB: ${ZNATIVE_LIB}")
    message(STATUS "ZNATIVE_LIBRARIES: ${ZNATIVE_LIBRARIES}")
    message(STATUS "ZNATIVE_INCLUDES: ${ZNATIVE_INCLUDES}")
    return()
endif ()

z_find_package_quiet(FOUND znative-static)
if (NOT FOUND)
    message(FATAL_ERROR "znative-static not found")
endif ()
set(ZNATIVE_LIB znative-static)

# 设置 ZNATIVE_INCLUDES 到 target znative-static
target_include_directories(znative-static INTERFACE ${ZNATIVE_INCLUDES})

z_get_target_dep_libs(znative-static ZNATIVE_LIBRARIES)
z_get_target_definitions(znative-static ZNATIVE_DEFINITIONS)

message(STATUS "ZNATIVE_LIBRARIES: ${ZNATIVE_LIBRARIES}")
# 遍历 ZNATIVE_LIBRARIES
foreach (libname ${ZNATIVE_LIBRARIES})
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

message(STATUS "ZNATIVE_LIB: ${ZNATIVE_LIB}")
message(STATUS "ZNATIVE_INCLUDES: ${ZNATIVE_INCLUDES}")
message(STATUS "ZNATIVE_DEFINITIONS: ${ZNATIVE_DEFINITIONS}")