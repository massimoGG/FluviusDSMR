cmake_minimum_required(VERSION 3.25)
project(DSMR VERSION 0.1.0 LANGUAGES C CXX)


add_compile_options(-Wall -Wextra -Werror)

add_executable(DSMR 
    common.c
    DSMR.cpp
    http.cpp
    influx.cpp
    tty.cpp
    main.cpp
)

target_compile_features(DSMR PUBLIC cxx_std_23)
