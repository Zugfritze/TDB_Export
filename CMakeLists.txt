cmake_minimum_required(VERSION 3.29)
project(TDB_Export)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ob3 /GL")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob3 /GL")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")

include_directories("externals/REFramework/include")

find_package(unordered_dense CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories("${CMAKE_BINARY_DIR}/header_template")
file(READ "src/init_schema.sql" SQL_initSchema)
configure_file("src/header_template/sql_resource.h.in" "header_template/sql_resource.h")

add_library(
        TDB_Export
        SHARED
        src/plugin.cpp
        src/type.hpp
        src/tool.hpp
)

set_property(
        TARGET
        TDB_Export
        PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(
        TDB_Export
        PRIVATE
        unordered_dense::unordered_dense
        unofficial::sqlite3::sqlite3
        SQLiteCpp
        spdlog::spdlog
)
