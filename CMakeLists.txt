cmake_minimum_required ( VERSION 2.6 )
project (koohar)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

  SET (warnings "-Wall -Wextra -Woverloaded-virtual -Werror")

  SET (common_flags "-std=c++0x -fPIC")

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
      ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    SET (clang_flags "-stdlib=libc++")
  endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR
         ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

  SET (CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${warnings} ${common_flags} ${clang_flags}"
  )

  SET (CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} ${warnings} ${common_flags} ${clang_flags} -g -ggdb"
  )

  SET (CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} ${warnings} ${common_flags} ${clang_flags} -O2"
  )

  SET (koohar_platform_base
    src/base/file_posix.cc
    src/base/filemapping_posix.cc
  )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  
  SET (koohar_platform_base
    src/base/file_win.cc
    src/base/filemapping_win.cc
  )

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

SET ( koohar_base
  ${koohar_platform_base}
  src/base/date.cc
  src/base/json.cc
  src/base/utils.cc
)

SET ( koohar_src
  ${koohar_base}
  src/client_response.cc
  src/client_request.cc
  src/http.cc
  src/http_parser.cc
  src/input_connection.cc
  src/output_connection.cc
  src/request.cc
  src/response.cc
  src/server_asio.cc
  src/server_config.cc
  src/static_transfer.cc
  src/uri_parser.cc
)

add_library ( koohar ${koohar_src} )
