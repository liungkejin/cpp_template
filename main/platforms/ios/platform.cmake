message(STATUS "====== iOS platform =====")

target_sources(${MAIN_TARGET} PRIVATE
        ${PLATFORM_SRC_DIR}/Platform.cpp
)
