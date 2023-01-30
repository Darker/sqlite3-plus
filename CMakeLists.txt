cmake_minimum_required(VERSION 3.5)

project(SQLITE++ VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SQLITE3_HOME "src/sqlite3/sqlite-amalgamation-3360000")

file(GLOB helloworld_SRC "src/**.cpp")
add_library( sqlite3++ STATIC
    ${helloworld_SRC}
    "${SQLITE3_HOME}/sqlite3.c"
)
target_include_directories( sqlite3++ PRIVATE include/sqlite3++ INTERFACE include PUBLIC ${SQLITE3_HOME} )

