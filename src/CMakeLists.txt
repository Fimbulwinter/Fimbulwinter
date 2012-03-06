


#
# setup and static libraries
#
add_subdirectory( Common )
option( BUILD_SQL_SERVERS "build sql server executables" ON )

#
# targets
#
add_subdirectory( AuthServer )
add_subdirectory( CharServer )
add_subdirectory( ZoneServer )



