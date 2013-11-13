include_directories( ../bitshares/fc/include )
include_directories( ../bitshares/include )
include_directories(../bitshares/vendor/leveldb-1.12.0/include)
include_directories(../sphlib-3.0/c)

add_executable( pool_miner miner.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c )
target_link_libraries( pool_miner  ${CMAKE_CURRENT_SOURCE_DIR}/sphlib-3.0/c/libsph.a fc ${BOOST_LIBRARIES} bshare leveldb ${BOOST_LIBRARIES} fc )
add_executable( pool_server server.cpp fast_momentum.cpp bitcoin.cpp sphlib-3.0/c/sha2big.c )
target_link_libraries( pool_server  ${CMAKE_CURRENT_SOURCE_DIR}/sphlib-3.0/c/libsph.a fc ${BOOST_LIBRARIES} bshare leveldb ${BOOST_LIBRARIES} fc )
