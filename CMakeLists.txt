cmake_minimum_required(VERSION 3.10)

# set the project name
project(Bayespp VERSION 2020.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated-copy -Wno-error=deprecated-copy")

set(Boost_USE_STATIC_LIBS ON) 
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(BayesFilter)

include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}Config.cmake
	INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}ConfigVersion.cmake
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY ExactVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}ConfigVersion.cmake
	${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}Config.cmake
        DESTINATION lib/cmake/${PROJECT_NAME} )


install(EXPORT ${PROJECT_NAME}Targets
	NAMESPACE Bayespp::
	DESTINATION lib/cmake/${PROJECT_NAME}
)
