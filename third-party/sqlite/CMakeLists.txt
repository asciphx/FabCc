cmake_minimum_required(VERSION 2.8)
project(sqlite LANGUAGES CXX)
add_library(sqlite STATIC sqlite3.c sqlite3.h)
