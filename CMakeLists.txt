cmake_minimum_required ( VERSION 2.6 )
project (koohar)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

  SET (warnings "-Wall -Wextra -Woverloaded-virtual -Werror")

  SET (common_flags "-std=c++0x -stdlib=libc++ -fPIC")

  SET (CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${warnings} ${common_flags}"
  )

  SET (CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} ${warnings} ${common_flags} -g -ggdb"
  )

  SET (CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} ${warnings} ${common_flags} -O2"
  )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
       ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
       ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

include_directories (
	/opt/local/include
)

endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

include_directories (
  ${CMAKE_SOURCE_DIR}/src
)

set ( koohar_base
  src/base/date.cc
  src/base/file.cc
  src/base/filemapping.cc
  src/base/json.cc
  src/base/utils.cc
)

set ( koohar_src
  ${koohar_base}
  src/http_connection.cc
  src/http_parser.cc
  src/request.cc
  src/response.cc
  src/server_asio.cc
  src/server_config.cc
  src/uri_parser.cc
)

add_library ( koohar ${koohar_src} )
