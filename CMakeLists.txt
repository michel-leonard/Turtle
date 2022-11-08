# ANALYSE SYNTAXIQUE EN 3-EME ANNEE DE LICENCE INFORMATIQUE
# Utiliser Flex et Bison pour générer un interpréteur de langage Turtle

cmake_minimum_required(VERSION 2.8)

project(TURTLE C)

if(NOT UNIX)
	SET(BISON_EXECUTABLE "..\\flex-bison-windows\\win_bison.exe")
	SET(FLEX_EXECUTABLE "..\\flex-bison-windows\\win_flex.exe")
	#Windows (with Flex and Bison installed)
endif()

find_package(BISON)
find_package(FLEX)

set(CMAKE_C_FLAGS "-Wall -std=c99 -O2 -g")

bison_target(turtle-parser
  turtle-parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/turtle-parser.c
  COMPILE_FLAGS "-v"
)

flex_target(turtle-lexer
  turtle-lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/turtle-lexer.c
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/turtle-lexer.h"
)

add_flex_bison_dependency(turtle-lexer turtle-parser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(turtle
  turtle.c
  turtle-ast.c
  ${BISON_turtle-parser_OUTPUTS}
  ${FLEX_turtle-lexer_OUTPUTS}
)

target_link_libraries(turtle m)

target_compile_definitions(turtle
  PRIVATE
    _POSIX_C_SOURCE=200809L
)
