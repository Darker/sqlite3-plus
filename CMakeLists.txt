cmake_minimum_required(VERSION 3.5)

project(SQLITE++ VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SQLITE3_HOME "src/sqlite3/sqlite-amalgamation-3360000")
set(SQLITE3_CORE_C "${SQLITE3_HOME}/sqlite3.c")

# set_source_files_properties(SQLITE3_CORE_C PROPERTIES LANGUAGE CXX )

file(GLOB sqlitepp_SRC "src/**.cpp")
file(GLOB sqlitepp_internal_SRC "src/internal/**.cpp")
add_library( sqlite3++ STATIC
    ${SQLITE3_CORE_C}
    ${sqlitepp_SRC}
    ${sqlitepp_internal_SRC}
)
target_include_directories( sqlite3++ PRIVATE include/sqlite3++ INTERFACE include PRIVATE ${SQLITE3_HOME} )

