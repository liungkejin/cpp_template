message(STATUS "====== Desktop platform =====")

if (WIN32)
    include(${MAIN_DIR}/cmake/platforms/Windows.cmake)
elseif (APPLE)
    include(${MAIN_DIR}/cmake/platforms/MacOS.cmake)
else ()
    include(${MAIN_DIR}/cmake/platforms/Linux.cmake)
endif ()