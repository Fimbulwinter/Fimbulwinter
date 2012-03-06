if( WIN32 )
message( STATUS "Detecting local SOCI" )
find_path( SOCI_DIR "soci.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include/soci"
	NO_DEFAULT_PATH )
find_library( SOCI_LOCAL_LIBRARIES
	NAMES libmysql
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( SOCI_LOCAL_LIBRARIES )
mark_as_advanced( SOCI_DIR )

if( SOCI_LOCAL_LIBRARIES AND SOCI_DIR )

	message( STATUS "Found SOCI: ${SOCI_LOCAL_LIBRARIES}" )
	
	set( HAVE_LOCAL_MYSQL ON
		CACHE BOOL "mysql client is available as a local copy")
	mark_as_advanced( HAVE_LOCAL_SOCI )
else()
	foreach( _VAR SOCI_LOCAL_LIBRARIES SOCI_DIR )
		if( NOT "${_VAR}" )
			set( MISSING_VARS ${MISSING_VARS} ${_VAR} )
		endif()
	endforeach()
	message( STATUS "Could NOT find SOCI (missing: ${MISSING_VARS})" )
	unset( HAVE_LOCAL_SOCI CACHE )
endif()
message( STATUS "Detecting local SOCI - done" )
endif( WIN32 )


#
# system
#
message( STATUS "Detecting system SOCI Library" )
unset( SOCI_LIBRARIES CACHE )
unset( SOCI_INCLUDE_DIRS CACHE )
find_package( Soci )
set( SOCI_SYSTEM_LIBRARIES "${SOCI_LIBRARIES}"
	CACHE PATH "system soci libraries" )
set( SOCI_DIR "${SOCI_INCLUDE_DIRS}"
	CACHE PATH "system soci include directories" )
mark_as_advanced( SOCI_SYSTEM_LIBRARIES )
mark_as_advanced( SOCI_DIR )

if( SOCI_SYSTEM_LIBRARIES AND SOCI_DIR )
	set( HAVE_SYSTEM_SOCI ON
		CACHE BOOL "SOCI client is available on the system" )
	mark_as_advanced( HAVE_SYSTEM_SOCI )
else()
	unset( HAVE_SYSTEM_SOCI CACHE )
endif()
message( STATUS "Detecting system SOCI - done" )


#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( Soci )
