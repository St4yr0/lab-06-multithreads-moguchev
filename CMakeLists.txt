cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("tools/gate/cmake/HunterGate.cmake")

huntergate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
  SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
)

project(template)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(GTest)
hunter_add_package(Boost COMPONENTS system filesystem thread log)
find_package(Boost CONFIG REQUIRED system filesystem thread log_setup log)
find_package(GTest CONFIG REQUIRED)

add_executable(tests main.cpp)

target_include_directories(tests 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third-party/picosha2
)
target_link_libraries(tests PUBLIC
  GTest::main 
  Boost::system
  Boost::thread
  Boost::log_setup
  Boost::log
  Boost::filesystem)

enable_testing()
add_test(NAME unit_tests COMMAND tests) 