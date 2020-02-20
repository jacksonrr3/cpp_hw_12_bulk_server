cmake_minimum_required(VERSION 3.12)

project(bulk_server VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Threads)
find_package(Boost COMPONENTS system REQUIRED)
                              
set(CMAKE_CXX_STANDARD 17)

add_executable(bulk_server src/main.cpp src/server.h)

add_library(async SHARED src/async.cpp src/async.h)

set_target_properties(async PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-fsanitize=thread"
    LINK_FLAGS "-fsanitize=thread"
)

set_target_properties(bulk_server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_DEFINITIONS BOOST_ASIO_ENABLE_HANDLER_TRACKING
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-fsanitize=address"
    LINK_FLAGS "-fsanitize=address"
)

target_link_libraries(async ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(bulk_server
        async
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )
        
install(TARGETS bulk_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT jacksonrr3@yandex.ru)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
