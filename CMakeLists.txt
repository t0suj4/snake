cmake_minimum_required (VERSION 2.6)
project (snake)
# Version number.
set (snake_VERSION_MAJOR 1)
set (snake_VERSION_MINOR 0)

configure_file (
	"${PROJECT_SOURCE_DIR}/snakeconfig.h.in"
	"${PROJECT_BINARY_DIR}/snakeconfig.h"
	)

include_directories("${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_LIBRARY_DIR}/list")
include_directories("${PROJECT_LIBRARY_DIR}/c11threads")

add_executable(snake snake.c)
