
include_directories(../sphlib-3.0/c)

SET (MINER_ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
SET (MINER_STATIC_BUILD 1)

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()
if( APPLE )
    SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
endif()

SET (CMAKE_FIND_LIBRARY_SUFFIXES ${MINER_ORIGINAL_LIB_SUFFIXES})

add_executable( pool_miner miner.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_miner bshare ${rt_library} ${CMAKE_DL_LIBS})
add_executable( pool_server server.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_server bshare ${rt_library} ${CMAKE_DL_LIBS})
