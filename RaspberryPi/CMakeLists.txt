cmake_minimum_required(VERSION 3.25)
project(DSMR VERSION 0.1.0 LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -Werror -fanalyzer)

add_executable(DSMR 
    common.c
    DSMR.c
    hash.c
    http.cpp
    influx.cpp
    tty.c
    main.cpp
)
