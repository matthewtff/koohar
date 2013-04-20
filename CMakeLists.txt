cmake_minimum_required ( VERSION 2.6 )
project (koohar)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

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
	
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

set ( koohar_src
	src/date.cc
	src/file.cc
	src/filemapping.cc
	src/http_connection.cc
	src/http_parser.cc
	src/parser.cc
	src/request.cc
	src/response.cc
	src/server_asio.cc
	src/server_config.cc
	src/uri_parser.cc
	src/webpage.cc
	src/webui.cc
	src/xmlobjects.cc
)

add_library ( koohar ${koohar_src} )
