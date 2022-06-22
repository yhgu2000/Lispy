cmake_minimum_required(VERSION 3.21)

project(Lispy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(BOOST_ROOT "D:/ETC/boost/1.76.0")
find_package(Boost 1.76 REQUIRED)

add_compile_definitions(BOOST_ALL_DYN_LINK)



file(GLOB _lispy_sources lispy/*)

add_library(lispy SHARED ${_lispy_sources})

target_link_libraries(lispy PUBLIC Boost::boost)

if(WIN32)
  target_compile_definitions(lispy
    INTERFACE "LISPY_EXPORT=__declspec(dllimport)"
    PRIVATE "LISPY_EXPORT=__declspec(dllexport)"
  )
else()
  target_compile_definitions(lispy "LISPY_EXPORT=")
endif()



add_executable(repl repl.cpp)

target_link_libraries(repl PUBLIC lispy)



add_executable(main main.cpp)

target_link_libraries(main PUBLIC lispy)
