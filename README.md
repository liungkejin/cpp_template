# thorvg-test

This project is to test the performance impact of caching GlProgram on GlCanvas in real-time rendering scenarios, and also to test two newly added interfaces:

* `tvg::Picture::load(uint32_t textureId, uint32_t width, uint32_t height)`
* `tvg::GlCanvas::target(int32_t fboId, uint32_t w, uint32_t h, int msaaSamples)`

## Project Structure

```text
samples/local/
    |-- assets/
    |-- libs/
         |-- glfw-3.4/
         |-- imgui-1.92.5/
         |-- thorvg/
    |-- src/
        |-- ThorvgTestWindow.cpp
    CMakeLists.txt
CMakeLists.txt
CMakePresets.json
run.sh
```

The test project is implemented based on imgui/glfw with an OpenGL backend and built using cmake. (thorvg is built using meson, but I am not very familiar with meson.) Therefore, thorvg is compiled first before building the project. So, if any modifications are made to the thorvg submodule, please delete the `thorvg/build/install` directory first and then recompile the project.

```cmake
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
```

It is recommended to open and run this project using CLion, or you can directly run run.sh (only supports macOS).