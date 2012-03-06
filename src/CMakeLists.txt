


#
# setup and static libraries
#
add_subdirectory( Common )
if( HAVE_common_sql )
	option( BUILD_SQL_SERVERS "build sql server executables" ON )
else()
	message( STATUS "Disabled sql server targets (requires common_sql)" )
endif()

#
# targets
#
add_subdirectory( AuthServer )
add_subdirectory( CharServer )
add_subdirectory( ZoneServer )



