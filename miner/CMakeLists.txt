
include_directories( ../bitshares/fc/include )
include_directories( ../bitshares/include )
include_directories(../bitshares/vendor/leveldb-1.12.0/include)
include_directories(../sphlib-3.0/c)

if(WIN32)
else(WIN32)
set(SSL_LIBS ssl crypto)
endif(WIN32)

add_executable( pool_miner miner.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_miner  ${SSL_LIBS} fc ${BOOST_LIBRARIES} bshare leveldb ${BOOST_LIBRARIES} fc )
add_executable( pool_server server.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c sha2.cpp )
target_link_libraries( pool_server  ${SSL_LIBS} fc ${BOOST_LIBRARIES} bshare leveldb ${BOOST_LIBRARIES} fc )
