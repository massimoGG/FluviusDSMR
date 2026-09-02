cmake_minimum_required(VERSION 3.25)

# For VScode
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

find_package(
    CURL
    REQUIRED
)

project(DSMR LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -Werror)

add_executable(DSMR 
    
    "1_LL/tty/tty.cpp"
    
    "2_/DSMR/DSMR.cpp"

    main.cpp
)

target_include_directories(DSMR PRIVATE
    .
)

add_subdirectory(../monorepo/common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(../monorepo/influx ${CMAKE_CURRENT_BINARY_DIR}/influx)

target_compile_features(DSMR PUBLIC cxx_std_23)
