cmake_minimum_required(VERSION 3.10)

# set the project name
project(Bayespp VERSION 2020.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_STATIC_LIBS ON) 
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(BayesFilter)


install(EXPORT Bayespp
  FILE
    BayesppTargets.cmake
  NAMESPACE
    Bayespp::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/Bayespp
)
