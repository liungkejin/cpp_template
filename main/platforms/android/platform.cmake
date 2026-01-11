message(STATUS "====== Android platform =====")

target_sources(${MAIN_TARGET} PRIVATE
        ${PLATFORM_SRC_DIR}/Platform.cpp
)

# support 16kb page size
target_link_options(${MAIN_TARGET} PUBLIC "-Wl,-z,max-page-size=16384")
target_link_options(${MAIN_TARGET} PUBLIC "-Wl,-z,common-page-size=16384")