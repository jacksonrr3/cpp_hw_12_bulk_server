cmake_minimum_required(VERSION 3.2)

project(bulk-server VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_library(async SHARED src/async.cpp src/async.h)

add_executable(bulk-server src/main src/server.h)

set_target_properties(bulk-server PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-fsanitize=thread"
    LINK_FLAGS "-fsanitize=thread"
      INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

set_target_properties(async PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-fsanitize=thread"
    LINK_FLAGS "-fsanitize=thread"
)

target_link_libraries(async ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(bulk-server async ${Boost_LIBRARIES})

install(TARGETS async LIBRARY DESTINATION lib)

install(TARGETS bulk-server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT jacksonrr3@yandex.ru)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
