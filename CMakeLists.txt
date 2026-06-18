cmake_minimum_required(VERSION 3.10)
project(Project1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(Project1
    main.c
    init.c
    dbAPI.c
    sqlite/sqlite3.c
)

if(APPLE)
    target_sources(Project1 PRIVATE platform/macOS/openURL.c)
elseif(WIN32)
    target_sources(Project1 PRIVATE platform/Windows/openURL.c)
else()
    target_sources(Project1 PRIVATE platform/Linux/openURL.c)
endif()

target_include_directories(Project1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
