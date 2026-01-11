
z_target_include_and_install_default(${MAIN_TARGET} "${PUBLIC_INCLUDE_DIR}/")

target_sources(${MAIN_TARGET} PRIVATE
        ${PUBLIC_SRC_DIR}/Public.cpp
)
