#
# common_base
#

set( COMMON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
	CACHE PATH "common source directory" )
mark_as_advanced( COMMON_SOURCE_DIR )

if( WITH_ZLIB )
message( STATUS "Creating target common_base" )
set( COMMON_HEADERS
	"${COMMON_SOURCE_DIR}/config_file.hpp"
	"${COMMON_SOURCE_DIR}/core.hpp"
	"${COMMON_SOURCE_DIR}/database_helper.h"
	"${COMMON_SOURCE_DIR}/datastruct.hpp"
	"${COMMON_SOURCE_DIR}/map_index.hpp"
	"${COMMON_SOURCE_DIR}/md5.hpp"
	"${COMMON_SOURCE_DIR}/packets.hpp"
	"${COMMON_SOURCE_DIR}/ragnarok.hpp"
	"${COMMON_SOURCE_DIR}/show_message.hpp"
	"${COMMON_SOURCE_DIR}/strfuncs.hpp"
	"${COMMON_SOURCE_DIR}/tcp_connection.hpp"
	"${COMMON_SOURCE_DIR}/tcp_server.hpp"
	"${COMMON_SOURCE_DIR}/timers.hpp"
	CACHE INTERNAL "common headers" )
set( COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/core.cpp"
	"${COMMON_SOURCE_DIR}/config_file.cpp"
	"${COMMON_SOURCE_DIR}/md5.cpp"
	"${COMMON_SOURCE_DIR}/show_message.cpp"
	"${COMMON_SOURCE_DIR}/strfuncs.cpp"
	"${COMMON_SOURCE_DIR}/tcp_connection.cpp"
	"${COMMON_SOURCE_DIR}/timers.cpp"
	CACHE INTERNAL "common sources" )

set( LIBRARIES ${ZLIB_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
add_library( common ${SOURCE_FILES} )
target_link_libraries( common ${LIBRARIES} )
set_target_properties( common PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} )
set( HAVE_common ON  CACHE INTERNAL "" )
set( TARGET_LIST ${TARGET_LIST} common  CACHE INTERNAL "" )
message( STATUS "Creating target common - done" )
else()
message( STATUS "Skipping target common (requires ZLIB)" )
unset( HAVE_common CACHE )
endif()



