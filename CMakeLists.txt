#####################################################################
#
# "Getting Started with CMake", a tutorial video by Eric Wing.
#   Part 1 of 6: http://www.youtube.com/watch?v=CLvZTyji_Uw
#   Part 2 of 6: http://www.youtube.com/watch?v=gUW-RrRQjEg
#   Part 3 of 6: http://www.youtube.com/watch?v=sz6cPhbuTk4
#   Part 4 of 6: http://www.youtube.com/watch?v=JICZOkyNXbg
#   Part 5 of 6: http://www.youtube.com/watch?v=lAiuLHy4dCk
#   Part 6 of 6: http://www.youtube.com/watch?v=fAtJNzDZdH8
#
# You can use notepad++ for syntax highlighting.
# Naming conventions:
#   WITH_*   : option to use an external package or not
#   ENABLE_* : option to use an internal feature/code or not
#   HAVE_*   : internal variable indicating if we have and are using something
#
#####################################################################


#cmake_minimum_required( VERSION 2.8.4 )
# Functional changes from 2.8.3 to 2.8.4:
#   string(SUBSTRING) works with length -1 as "rest of string"
#   changes to some CPack generators
#   CYGWIN no longer defines WIN32
#   CMP0017: Prefer files from the CMake module directory when including from there.
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
cmake_minimum_required( VERSION 2.8.3 )
project( Fimbulwinter CXX C)
if( CYGWIN )
	unset( WIN32 )
endif()


if( ALLOW_SAME_DIRECTORY )
elseif( "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}" )
	option( ALLOW_SAME_DIRECTORY "Allow CMake to build in the source directory." OFF )
	message( FATAL_ERROR
		"Do not use the source directory to build your files, instead delete CMakeCache.txt, create a separate folder and build there.\n"
		"Example: (build in subdir 'build' and install to source dir)\n"
		"  rm -f CMakeCache.txt\n"
		"  mkdir build\n"
		"  cd build\n"
		"  cmake -G\"Unix Makefiles\" -DINSTALL_TO_SOURCE=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..\n"
		"  make install\n"
		"  cd ..\n"
		"  rm -rf build\n"
		"To skip this check, set ALLOW_SAME_DIRECTORY to ON (-DALLOW_SAME_DIRECTORY=ON)" )
endif()

#
# Global stuff
#
set( GLOBAL_LIBRARIES ${LINK_LIBRARIES}  CACHE INTERNAL "" )# list (comma separated values)
set( GLOBAL_INCLUDE_DIRS ${INCLUDE_DIRECTORIES}  CACHE INTERNAL "" )# list (comma separated values)
set( GLOBAL_DEFINITIONS ${COMPILE_DEFINITIONS}  CACHE INTERNAL "" )# string (space separated values -DFOO=bar)
mark_as_advanced( GLOBAL_LIBRARIES  GLOBAL_INCLUDE_DIRS  GLOBAL_DEFINITIONS )
if( WIN32 )
	set_property( CACHE GLOBAL_DEFINITIONS  PROPERTY VALUE "${GLOBAL_DEFINITIONS} -DFD_SETSIZE=4096" )
endif()
if( MSVC )
	set_property( CACHE GLOBAL_LIBRARIES    PROPERTY VALUE ${GLOBAL_LIBRARIES} "oldnames.lib" "ws2_32.lib" )
	set_property( CACHE GLOBAL_DEFINITIONS  PROPERTY VALUE "${GLOBAL_DEFINITIONS} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE" )
endif()


#
# 3rd party
#
add_subdirectory( 3rdparty )
include( CheckCSourceCompiles )
include( CheckCSourceRuns )
include( CheckIncludeFile )
include( FindFunctionLibrary )
include( TestBigEndian )

#
# dynamic loading library (Linux)
#
if( NOT WIN32 )
message( STATUS "Detecting dynamic loading library (dl)" )
set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} )
find_function_library( dlopen FUNCTION_DLOPEN_LIBRARIES dl )
if( FUNCTION_DLOPEN_LIBRARIES )
	message( STATUS "Adding global library: ${FUNCTION_DLOPEN_LIBRARIES}" )
	set_property( CACHE GLOBAL_LIBRARIES  PROPERTY VALUE ${GLOBAL_LIBRARIES} ${FUNCTION_DLOPEN_LIBRARIES} )
endif()
message( STATUS "Detecting dynamic loading library (dl) - done" )
endif()

#
# Find Boost Library
#
FIND_PACKAGE(Boost)
     message( STATUS "Detecting Boost Library")
IF (Boost_FOUND)
    message( STATUS "Boost Library - Found")
    include_directories(${Boost_INCLUDE_DIRS})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
else()
    message( FATAL ERROR "Boost Library - Not Found")
ENDIF()

#
# networking library (Solaris/MinGW)
#
if( NOT MSVC )
message( STATUS "Detecting networking library (socket/nsl/ws2_32)" )
set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} )
find_function_library( bind FUNCTION_BIND_LIBRARIES socket ws2_32 )
if( FUNCTION_BIND_LIBRARIES )
	message( STATUS "Adding global library: ${FUNCTION_BIND_LIBRARIES}" )
	set_property( CACHE GLOBAL_LIBRARIES  PROPERTY VALUE ${GLOBAL_LIBRARIES} ${FUNCTION_BIND_LIBRARIES} )
endif()
set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} )
find_function_library( gethostbyname FUNCTION_GETHOSTBYNAME_LIBRARIES nsl )
if( FUNCTION_GETHOSTBYNAME_LIBRARIES )
	message( STATUS "Adding global library: ${FUNCTION_GETHOSTBYNAME_LIBRARIES}" )
	set_property( CACHE GLOBAL_LIBRARIES  PROPERTY VALUE ${GLOBAL_LIBRARIES} ${FUNCTION_GETHOSTBYNAME_LIBRARIES} )
endif()
message( STATUS "Detecting networking library (socket/nsl/ws2_32) - done" )
endif()


#
# Test for big endian
#
TEST_BIG_ENDIAN( BIG_ENDIAN )
if( NOT DEFINED BIG_ENDIAN )
	message( WARNING "unable to determine endianess, only LITTLE ENDIAN is supported" )
elseif( BIG_ENDIAN )
	message( FATAL_ERROR "bigendian is not supported" )
endif()


#####################################################################
# package stuff
#
set( CPACK_PACKAGE_NAME "Fimbulwinter" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "MMORPG server package" )
set( CPACK_PACKAGE_VERSION "Alpha" )
set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/licence.txt )
set( CMAKE_CXX_FLAGS "-pthread -c" )
set( CMAKE_EXE_LINKER_FLAGS "-o -lm" )
#set( CPACK_MONOLITHIC_INSTALL ON )
include( CPACK  OPTIONAL  RESULT_VARIABLE HAVE_CPACK )
if( HAVE_CPACK )
	option( WITH_CPACK "enable building packages with CPack ('package' target)" ON )
endif()
if( NOT WITH_CPACK )
	# empty replacements
	macro( cpack_add_component_group )
	endmacro()
	macro( cpack_add_component )
	endmacro()
	message( STATUS "Disabled package creation" )
endif()

set( Runtime "Runtime files"  CACHE INTERNAL "" )
set( Runtime_base "configurations, dbs, npcs, docs, ..."  CACHE INTERNAL "" )
cpack_add_component_group( Runtime DESCRIPTION ${Runtime} DISPLAY_NAME "Runtime" )
cpack_add_component( Runtime_base DESCRIPTION ${Runtime_base} DISPLAY_NAME "Base files" GROUP Runtime )

set( Development "Development files"  CACHE INTERNAL "" )
set( Development_base "projects, 3rdparty, sources "  CACHE INTERNAL "" )
cpack_add_component_group( Development DESCRIPTION ${Development} DISPLAY_NAME "Development" )
cpack_add_component( Development_base DESCRIPTION ${Development_base} DISPLAY_NAME "Base files" GROUP Development )


#
# install stuff
#
option( INSTALL_COMPONENT_RUNTIME "install/package files needed to run the project" ON )
option( INSTALL_COMPONENT_DEVELOPMENT "install/package files needed to build the project" OFF )
option( INSTALL_TO_PATH "copy files to INSTALL_PATH" OFF )
option( INSTALL_TO_SOURCE "copy files to source directory, skips what is already there (${CMAKE_CURRENT_SOURCE_DIR})" OFF )
option( INSTALL_TO_SUBDIR "copy files to subdirectory (${CMAKE_CURRENT_BINARY_DIR}/install)" OFF )
set( INSTALL_PATH "${CMAKE_INSTALL_PREFIX}"  CACHE STRING "install path (only used when INSTALL_TO_PATH is set)" )
mark_as_advanced( CMAKE_INSTALL_PREFIX )
if( INSTALL_TO_PATH AND NOT ("${INSTALL_TO}" STREQUAL "path") )# changed to path
	set_property( CACHE INSTALL_TO_SOURCE INSTALL_TO_SUBDIR  PROPERTY VALUE OFF )
elseif( INSTALL_TO_SOURCE AND NOT ("${INSTALL_TO}" STREQUAL "source") )# changed to source
	set_property( CACHE INSTALL_TO_PATH INSTALL_TO_SUBDIR  PROPERTY VALUE OFF )
elseif( INSTALL_TO_SUBDIR AND NOT ("${INSTALL_TO}" STREQUAL "subdir") )# changed to subdir
	set_property( CACHE INSTALL_TO_PATH INSTALL_TO_SOURCE  PROPERTY VALUE OFF )
elseif( NOT INSTALL_TO_PATH AND NOT INSTALL_TO_SOURCE AND NOT INSTALL_TO_SUBDIR )# default
	set_property( CACHE INSTALL_TO_SUBDIR  PROPERTY VALUE ON )
endif()
if( INSTALL_TO_PATH )
	set( INSTALL_TO "path"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${INSTALL_PATH}" )
elseif( INSTALL_TO_SOURCE )
	set( INSTALL_TO "source"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${CMAKE_CURRENT_SOURCE_DIR}" )
elseif( INSTALL_TO_SUBDIR )
	set( INSTALL_TO "subdir"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${CMAKE_CURRENT_BINARY_DIR}/install" )
endif()

set( DEVELOPMENT_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/fimbulvetr.sln"
	)

set( DEVELOPMENT_DIRECTORIES
	"3rdparty"
	"src"
	)

set( RUNTIME_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/soci_core_3_1.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/soci_mysql_3_1.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/libmysql.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/zlib1.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/licence.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/teamandthanks.txt"
	)

set( RUNTIME_DIRECTORIES
	"Config"
	"Data"
	)

if( INSTALL_TO_SOURCE )# skip, already in the source dir
else()
	if( INSTALL_COMPONENT_RUNTIME )
		install( FILES ${RUNTIME_FILES}
			DESTINATION "."
			COMPONENT Runtime_base )
		foreach( DIR IN ITEMS ${RUNTIME_DIRECTORIES} )
			install( DIRECTORY "${DIR}/"
				DESTINATION "${DIR}"
				COMPONENT Runtime_base
				)
		endforeach()
	endif( INSTALL_COMPONENT_RUNTIME )
	if( INSTALL_COMPONENT_DEVELOPMENT )
		install( FILES ${DEVELOPMENT_FILES}
			DESTINATION "."
			COMPONENT Development_base )
		foreach( DIR IN ITEMS ${DEVELOPMENT_DIRECTORIES} )
			install( DIRECTORY "${DIR}/"
				DESTINATION "${DIR}"
				COMPONENT Development_base
				)
		endforeach()
	endif( INSTALL_COMPONENT_DEVELOPMENT )
endif()

#
# sources
#
set( TARGET_LIST  CACHE INTERNAL "" )
add_subdirectory( src )


#####################################################################
# final checks and warnings
#
list( LENGTH  TARGET_LIST  _LEN )
if( _LEN EQUAL 0 )
	message( FATAL_ERROR "no targets available" )
endif()
message( STATUS "Available targets:" )
foreach( _TARGET IN ITEMS ${TARGET_LIST} )
	message( STATUS "\t${_TARGET}" )
endforeach()
