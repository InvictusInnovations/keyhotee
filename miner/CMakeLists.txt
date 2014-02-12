
include_directories( ../bitshares/fc/include )
include_directories( ../bitshares/include )
include_directories(../bitshares/vendor/leveldb-1.12.0/include)
include_directories(../sphlib-3.0/c)

SET (MINER_ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
SET (MINER_STATIC_BUILD 1)

if(WIN32)
  #You need to set OPENSSL_ROOT environment variable for your system on WIN32
  message("Setting up OpenSSL root and include vars on Win32 platform")
  set( OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT} )
  set( OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include )
ELSE()
  IF(MINER_STATIC_BUILD EQUAL 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
  ENDIF()

  if( NOT APPLE )  
    set(rt_library rt )
  endif() 
endif(WIN32)

find_package( OpenSSL )

SET (CMAKE_FIND_LIBRARY_SUFFIXES ${MINER_ORIGINAL_LIB_SUFFIXES})

add_executable( pool_miner miner.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_miner bshare leveldb fc ${BOOST_LIBRARIES}  ${OPENSSL_LIBRARIES} ${rt_library} ${CMAKE_DL_LIBS})
add_executable( pool_server server.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_server fc bshare leveldb fc ${BOOST_LIBRARIES} ${OPENSSL_LIBRARIES} ${rt_library} ${CMAKE_DL_LIBS})


