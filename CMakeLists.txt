cmake_minimum_required(VERSION 3.22)
project(WebHTMLServer C)

set(CMAKE_C_STANDARD 11)

add_executable(WebHTMLServer main.c csapp.c webserver.h csapp.h)
