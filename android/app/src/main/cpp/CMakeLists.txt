cmake_minimum_required(VERSION 3.22.1)

project("matrixrain")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../../.. matrix_rain_core)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp)

target_link_options(${CMAKE_PROJECT_NAME}
        PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384")

target_link_libraries(${CMAKE_PROJECT_NAME}
        MatrixRainCore
        android
        log)
