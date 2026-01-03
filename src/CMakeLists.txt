# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(error22_einval_library VERSION 0.0.1 LANGUAGES C CXX)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h *.hpp)
add_library(error22_einval SHARED
  ${SOURCES}
)



if(NOT EXISTS "${CMAKE_SOURCE_DIR}/../../extern/onnxruntime/cmake/CMakeLists.txt")
    message(FATAL_ERROR "extern/onnxruntime submodule 未初始化！")
endif()

set(ORT_PREBUILT_DIR "${CMAKE_SOURCE_DIR}/../../extern/onnxruntime/build/${CMAKE_SYSTEM_NAME}/Release")
set(ORT_LIB "${ORT_PREBUILT_DIR}/libonnxruntime.a")

#  若产物不存在 → 自动跑 build.sh
if(NOT EXISTS "${ORT_LIB}")
    message(STATUS "ONNX Runtime prebuilt lib missing → running build.sh ...")
    execute_process(
    COMMAND bash "${CMAKE_SOURCE_DIR}/../../extern/onnxruntime/build.sh" --config Release --parallel --compile_no_warning_as_error --skip_tests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../"
    RESULT_VARIABLE BUILD_RESULT
)
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "build.sh failed, exit code ${BUILD_RESULT}")
    endif()
endif()
set_target_properties(onnxruntime  IMPORTED_LOCATION "${ORT_LIB}")

add_library(onnxruntime STATIC IMPORTED GLOBAL)

target_link_libraries(error22_einval PRIVATE onnxruntime)

target_include_directories(error22_einval PRIVATE
  "${CMAKE_SOURCE_DIR}/../../extern/onnxruntime/include/"
)


set_target_properties(error22_einval PROPERTIES
  PUBLIC_HEADER error22_einval.h
  OUTPUT_NAME "error22_einval"
)

target_compile_definitions(error22_einval PUBLIC DART_SHARED_LIB)
if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(error22_einval PRIVATE "-Wl,-z,max-page-size=16384")
endif()
set(CMAKE_CXX_STANDARD 20)     
set(CMAKE_CXX_STANDARD_REQUIRED ON)

