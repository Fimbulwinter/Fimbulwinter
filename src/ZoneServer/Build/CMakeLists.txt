#
# login sql
#
if( BUILD_SQL_SERVERS )
message( STATUS "Creating target ZoneServer" )
set( ZONESERVER_HEADERS
	"${ZONESERVER_SOURCE_DIR}/BlockManager.hpp"
	"${ZONESERVER_SOURCE_DIR}/item_db.hpp"
	"${ZONESERVER_SOURCE_DIR}/MapManager.hpp"
	"${ZONESERVER_SOURCE_DIR}/PacketHandling.hpp"
	"${ZONESERVER_SOURCE_DIR}/PlayerModules.hpp"
	"${ZONESERVER_SOURCE_DIR}/ZoneServer.hpp"
	)

set( ZONESERVER_SOURCES
	"${ZONESERVER_SOURCE_DIR}/BlockManager.cpp"
	"${ZONESERVER_SOURCE_DIR}/client.cpp"
	"${ZONESERVER_SOURCE_DIR}/inter_char.cpp"
	"${ZONESERVER_SOURCE_DIR}/item_db.cpp"
	"${ZONESERVER_SOURCE_DIR}/MapManager.cpp"
	"${ZONESERVER_SOURCE_DIR}/PacketHandling.cpp"
	"${ZONESERVER_SOURCE_DIR}/PlayerModules.cpp"
	"${ZONESERVER_SOURCE_DIR}/ZoneServer.cpp"
	)

set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DWITH_SQL" )
set( SOURCE_FILES ${COMMON_HEADERS} ${ZONESERVER_HEADERS} ${ZONESERVER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} )
source_group( Zone FILES ${ZONESERVER_HEADERS} ${ZONESERVER_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( ZoneServer ${SOURCE_FILES} )
add_dependencies( ZoneServer ${DEPENDENCIES} )
target_link_libraries( ZoneServer ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( ZoneServer PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_ZoneServer DESCRIPTION "Zone Server" DISPLAY_NAME "ZoneServer" GROUP Runtime )
	install( TARGETS ZoneServer
		DESTINATION "."
		COMPONENT Runtime_ZoneServer )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} ZoneServer  CACHE INTERNAL "" )
message( STATUS "Creating target ZoneServer - done" )
endif( BUILD_SQL_SERVERS )
