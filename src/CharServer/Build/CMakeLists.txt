#
# login sql
#
if( BUILD_SQL_SERVERS )
message( STATUS "Creating target CharServer" )
set( CHARSERVER_HEADERS
	"${CHARSERVER_SOURCE_DIR}/CharServer.hpp"
	"${CHARSERVER_SOURCE_DIR}/CharDB.hpp"
	)

set( CHARSERVER_SOURCES
	"${CHARSERVER_SOURCE_DIR}/inter_auth.cpp"
	"${CHARSERVER_SOURCE_DIR}/inter_zone.cpp"
	"${CHARSERVER_SOURCE_DIR}/client.cpp"
	"${CHARSERVER_SOURCE_DIR}/CharServer.cpp"
	)

set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${Boost_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DWITH_SQL -DHAS_BOOST" )
set( SOURCE_FILES ${COMMON_HEADERS} ${CHARSERVER_HEADERS} ${CHARSERVER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} )
source_group( Auth FILES ${CHARSERVER_HEADERS} ${CHARSERVER_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( CharServer ${SOURCE_FILES} )
add_dependencies( CharServer ${DEPENDENCIES} )
target_link_libraries( CharServer ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( CharServer PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_CharServer DESCRIPTION "Characters Server" DISPLAY_NAME "CharServer" GROUP Runtime )
	install( TARGETS CharServer
		DESTINATION "."
		COMPONENT Runtime_CharServer )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} CharServer  CACHE INTERNAL "" )
message( STATUS "Creating target CharServer - done" )
endif( BUILD_SQL_SERVERS )
