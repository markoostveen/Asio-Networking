cmake_minimum_required (VERSION 3.8)

project("AsioNetworking")


file(GLOB_RECURSE  Sources
  "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
)

find_package(Threads REQUIRED)

add_library(AsioNetworking STATIC ${Sources})
target_include_directories(AsioNetworking PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Src")
set_property(TARGET AsioNetworking PROPERTY CXX_STANDARD 17)
target_link_libraries(AsioNetworking PUBLIC Threads::Threads)

target_include_directories(AsioNetworking PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/asio-1.18.1/include")

add_executable(TestProgram "${CMAKE_CURRENT_SOURCE_DIR}/Tests/main.cpp")
target_link_libraries(TestProgram PUBLIC AsioNetworking)
set_property(TARGET TestProgram PROPERTY CXX_STANDARD 17)