#
# system
#
#
# Find SQLite Library
#
message( STATUS "Detecting system SQLite" )
FIND_PACKAGE(Sqlite3)
IF (SQLITE3_FOUND)
    message( STATUS "SQLite Library Found!")
    INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_SQLite" )
else()
    message( FATAL ERROR "SQLite Library Not Found!")
ENDIF()

#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( SQLite3 )
