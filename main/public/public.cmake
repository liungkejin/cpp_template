
# tinyfmt
z_target_include_public_build_interface_default(${MAIN_TARGET} ${PUBLIC_LIBS_DIR}/tinyformat/)

target_sources(${MAIN_TARGET} PRIVATE
        ${PUBLIC_SRC_DIR}/Public.cpp
        ${PUBLIC_SRC_DIR}/ZLog.cpp
)
