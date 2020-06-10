cmake_minimum_required(VERSION 3.10)

set(BayesFilterHeaders
	allFilters.hpp
	bayesFlt.hpp
	CIFlt.hpp
	# compatibility.hpp
	covFlt.hpp
	infFlt.hpp
	infRtFlt.hpp
	itrFlt.hpp
	matSup.hpp
	matSupSub.hpp
	models.hpp
	schemeFlt.hpp
	SIRFlt.hpp
	uBLASmatrix.hpp
	UDFlt.hpp
	uLAPACK.hpp
	unsFlt.hpp
)
set(BayesFilterFiltersHeaders
	filters/average1.hpp
	filters/indirect.hpp
)

add_library(BayesFilter STATIC 
	${BayesFilterHeaders}
	${BayesFilterFiltersHeaders}
	bayesFlt.cpp
	bayesFltAlg.cpp
	CIFlt.cpp
	covFlt.cpp
	infFlt.cpp
	infRtFlt.cpp
	itrFlt.cpp
	matSup.cpp
	SIRFlt.cpp
	UDFlt.cpp
	UdU.cpp
	unsFlt.cpp
)

target_compile_options(BayesFilter PRIVATE -D_GLIBCXX_USE_CXX11_ABI=1 -Wall -Werror -Wextra -pedantic-errors)

include(GNUInstallDirs)

install(TARGETS BayesFilter
	EXPORT Bayespp
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	INCLUDES DESTINATION "include"
)

install(
	FILES  ${BayesFilterHeaders}
	DESTINATION "include/BayesFilter"
)

install(
	FILES  ${BayesFilterFiltersHeaders}
	DESTINATION "include/BayesFilter/filters"
)


