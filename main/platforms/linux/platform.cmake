message(STATUS "====== Linux platform =====")

target_sources(${MAIN_TARGET} PRIVATE
        ${PLATFORM_SRC_DIR}/Platform.cpp
)
