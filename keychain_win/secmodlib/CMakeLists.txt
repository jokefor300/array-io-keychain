project( secmodlib  )

cmake_minimum_required( VERSION 3.0.2 )

set(CMAKE_CXX_FLAGS "-std=c++11 /EHsc")

SET(BOOST_COMPONENTS)
LIST(APPEND BOOST_COMPONENTS log)
SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )

SET(BOOST_ROOT $ENV{BOOST_ROOT})
set(Boost_USE_MULTITHREADED ON)
set(BOOST_ALL_DYN_LINK OFF)

FIND_PACKAGE(Boost 1.65 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
SET(BOOST_LIBRARIES_TEMP ${Boost_LIBRARIES})
FIND_PACKAGE(Boost 1.65 REQUIRED COMPONENTS coroutine)
LIST(APPEND BOOST_COMPONENTS coroutine)
SET(Boost_LIBRARIES ${BOOST_LIBRARIES_TEMP} ${Boost_LIBRARIES})

add_definitions(-DUNICODE -D_UNICODE)

include_directories(
		"${CMAKE_SOURCE_DIR}/keychain_win/keychain_agent/include"
		"${CMAKE_SOURCE_DIR}/keychain_win/secmodlib/include"
		"${CMAKE_SOURCE_DIR}/keychain_lib/include"
	    "${CMAKE_SOURCE_DIR}/libraries/fc_light/include"
		"${CMAKE_SOURCE_DIR}/libraries/eth-crypto/include"
		"${CMAKE_SOURCE_DIR}/libraries/secp256k1/include"
		"${CMAKE_SOURCE_DIR}/libraries/secp256k1_ext/include"
		"${CMAKE_SOURCE_DIR}/libraries/kaitai_struct_cpp_stl_runtime"
        "${OPENSSL_ROOT_DIR}/include"
        ${Boost_INCLUDE_DIR})

if( MSVC )
	add_definitions( -DNOMINMAX )
endif()

file(GLOB KEYCHAIN_SECMOD_SOURCES "./src/*.cpp")
file(GLOB KEYCHAIN_SECMOD_INCLUDES "./include/keychain_lib/*.hpp" 	"../keychain_win/secmodlib/include/*.hpp")
add_library(secmodlib ${KEYCHAIN_SECMOD_SOURCES})