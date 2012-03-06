#
# login sql
#
if( BUILD_SQL_SERVERS )
message( STATUS "Creating target AuthServer" )
set( AUTHSERVER_HEADERS
	"${AUTHSERVER_SOURCE_DIR}/AccountDB.h"
	"${AUTHSERVER_SOURCE_DIR}/AuthServer.hpp"
	)

set( AUTHSERVER_SOURCES
	"${AUTHSERVER_SOURCE_DIR}/inter.cpp"
	"${AUTHSERVER_SOURCE_DIR}/client.cpp"
	"${AUTHSERVER_SOURCE_DIR}/auth.cpp"
	)

set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${Boost_LIBRARIES} ${SOCI_LIBRARIES}  )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${SOCI_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DWITH_SQL" )
set( SOURCE_FILES ${COMMON_HEADERS} ${AUTHSERVER_HEADERS} ${AUTHSERVER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} )
source_group( Auth FILES ${AUTHSERVER_HEADERS} ${AUTHSERVER_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( AuthServer ${SOURCE_FILES} )
add_dependencies( AuthServer ${DEPENDENCIES} )
target_link_libraries( AuthServer ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( AuthServer PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_AuthServer DESCRIPTION "Authentication Server" DISPLAY_NAME "AuthServer" GROUP Runtime )
	install( TARGETS AuthServer
		DESTINATION "."
		COMPONENT Runtime_AuthServer )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} AuthServer  CACHE INTERNAL "" )
message( STATUS "Creating target AuthServer - done" )
endif( BUILD_SQL_SERVERS )
