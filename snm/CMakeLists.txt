cmake_minimum_required(VERSION 3.13)
project(gatesnm VERSION 1.0 LANGUAGES C)

add_library(gatesnm snm.c snm.h)
target_include_directories(gatesnm PUBLIC .)
