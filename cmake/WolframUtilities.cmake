# WolframUtilities.cmake
#
# Set of short utility functions that may be helpful for Mathematica paclets that use CMake
#
# Author: Rafal Chojna - rafalc@wolfram.com

function(get_default_mathematica_dir MATHEMATICA_VERSION DEFAULT_MATHEMATICA_INSTALL_DIR)
	set(_M_INSTALL_DIR NOTFOUND)
	if(APPLE)
	 	find_path(_M_INSTALL_DIR "Contents" PATHS 
			"/Applications/Mathematica ${MATHEMATICA_VERSION}.app"
			"/Applications/Mathematica.app"
		)
		set(_M_INSTALL_DIR "${_M_INSTALL_DIR}/Contents")
	elseif(WIN32)
		set(_M_INSTALL_DIR "C:/Program\ Files/Wolfram\ Research/Mathematica/${MATHEMATICA_VERSION}")
	else()
		set(_M_INSTALL_DIR "/usr/local/Wolfram/Mathematica/${MATHEMATICA_VERSION}")
	endif()
	if(NOT IS_DIRECTORY "${_M_INSTALL_DIR}" AND IS_DIRECTORY "$ENV{MATHEMATICA_HOME}")
		set(_M_INSTALL_DIR "$ENV{MATHEMATICA_HOME}")
	endif()
	set(${DEFAULT_MATHEMATICA_INSTALL_DIR} "${_M_INSTALL_DIR}" PARENT_SCOPE)
endfunction()

function(detect_system_id DETECTED_SYSTEM_ID)
	if(NOT ${DETECTED_SYSTEM_ID})
		#set system id and build platform
		set(BITNESS 32)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(BITNESS 64)
		endif()

		set(INITIAL_SYSTEMID NOTFOUND)

		# Determine the current machine's systemid.
		if(CMAKE_C_COMPILER MATCHES "androideabi")
			set(INITIAL_SYSTEMID Android)
		elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
			set(INITIAL_SYSTEMID Linux-ARM)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND BITNESS EQUAL 64)
			set(INITIAL_SYSTEMID Linux-x86-64)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND BITNESS EQUAL 32)
			set(INITIAL_SYSTEMID Linux)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND BITNESS EQUAL 64)
			set(INITIAL_SYSTEMID Windows-x86-64)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND BITNESS EQUAL 32)
			set(INITIAL_SYSTEMID Windows)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND BITNESS EQUAL 64)
			set(INITIAL_SYSTEMID MacOSX-x86-64)
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND BITNESS EQUAL 32)
			set(INITIAL_SYSTEMID MacOSX-x86)
		endif()

		if(NOT INITIAL_SYSTEMID)
			message(FATAL_ERROR "Unable to determine System ID.")
		endif()

		set(${DETECTED_SYSTEM_ID} "${INITIAL_SYSTEMID}" PARENT_SCOPE)
	endif()
endfunction()

function(detect_build_platform DETECTED_BUILD_PLATFORM)
	# Determine the current machine's build platform.
	set(BUILD_PLATFORM Indeterminate)
	if(CMAKE_SYSTEM_NAME STREQUAL "Android")
		if(CMAKE_C_COMPILER_VERSION VERSION_LESS 4.9)
			set(BUILD_PLATFORM_ERROR "Android build with gcc version less than 4.9")
		else()
			set(BUILD_PLATFORM android-16-gcc4.9)
		endif()
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm*" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
		if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 4.7)
			set(BUILD_PLATFORM_ERROR "Arm build with gcc less than 4.7")
		elseif(CMAKE_C_COMPILER AND NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
			set(BUILD_PLATFORM_ERROR "Arm build with non-gnu compiler")
		else()
			#at some point might be smart to dynamically construct this build platform, but
			#for now it's all we build ARM on so it should be okay
			set(BUILD_PLATFORM armv6-glibc2.19-gcc4.9)
		endif()
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 5.2)
			set(BUILD_PLATFORM_ERROR "Linux build with gcc less than 5.2")
		elseif(CMAKE_C_COMPILER AND NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
			set(BUILD_PLATFORM_ERROR "Linux build with non-gnu compiler")
		else()
			set(BUILD_PLATFORM scientific6-gcc4.8)
		endif()
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		if((NOT CMAKE_C_COMPILER) OR (NOT MSVC_VERSION LESS 1900))
			if(MSVC_VERSION EQUAL 1900)
				set(BUILD_PLATFORM vc140)
			elseif(MSVC_VERSION GREATER_EQUAL 1910)
				set(BUILD_PLATFORM vc141)
			endif()
		else()
			set(BUILD_PLATFORM_ERROR "Windows build without VS 2015 or greater.")
		endif()
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		if(CMAKE_SYSTEM_VERSION VERSION_LESS 10.9)
			set(BUILD_PLATFORM_ERROR "OSX build on OSX less than 10.9")
		else()
			set(BUILD_PLATFORM libcxx-min10.9)
		endif()
	else()
		set(BUILD_PLATFORM_ERROR "Unrecognized system type.")
	endif()

	if(BUILD_PLATFORM STREQUAL "Indeterminate")
		message(FATAL_ERROR "Unable to determine Build Platform. Reason: ${BUILD_PLATFORM_ERROR}")
	endif()

	set(${DETECTED_BUILD_PLATFORM} "${BUILD_PLATFORM}" PARENT_SCOPE)
endfunction()

#set MathLink library name depending on the platform
function(get_mathlink_library_name MATHLINK_INTERFACE_VERSION MATHLINK_LIB_NAME)

	detect_system_id(SYSTEM_ID)

	set(MATHLINK_LIBRARY NOTFOUND)
	if(SYSTEM_ID STREQUAL "MacOSX-x86-64")
		set(MATHLINK_LIBRARY "MLi${MATHLINK_INTERFACE_VERSION}")
	elseif(SYSTEM_ID STREQUAL "Linux" OR SYSTEM_ID STREQUAL "Linux-ARM" OR SYSTEM_ID STREQUAL "Windows")
		set(MATHLINK_LIBRARY "ML32i${MATHLINK_INTERFACE_VERSION}")
	elseif(SYSTEM_ID STREQUAL "Linux-x86-64" OR SYSTEM_ID STREQUAL "Windows-x86-64")
		set(MATHLINK_LIBRARY "ML64i${MATHLINK_INTERFACE_VERSION}")
	endif()

	if(NOT MATHLINK_LIBRARY)
		message(FATAL_ERROR "Unable to determine MathLink library name for system: ${SYSTEM_ID}")
	endif()

	set(${MATHLINK_LIB_NAME} "${MATHLINK_LIBRARY}" PARENT_SCOPE)
endfunction()

# not sure if this one is needed, keep it just in case
function(additional_paclet_dependencies SYSTEM_ID EXTRA_LIBS)
	if (${SYSTEM_ID} STREQUAL "MacOSX-x86-64")
		set(EXTRA_LIBS "c++" "-framework Foundation" PARENT_SCOPE)
	elseif (${SYSTEM_ID} STREQUAL "Linux")
		# nothing for now
	elseif (${SYSTEM_ID} STREQUAL "Linux-x86-64")
		# nothing for now
	elseif (${SYSTEM_ID} STREQUAL "Linux-ARM")
		# nothing for now
	elseif (${SYSTEM_ID} STREQUAL "Windows")
		# nothing for now
	elseif (${SYSTEM_ID} STREQUAL "Windows-x86-64")
		# nothing for now
	endif ()
endfunction()

# set machine bitness flags for given target
function(set_machine_flags TARGET_NAME)
	detect_system_id(SYSTEM_ID)

	if(SYSTEM_ID MATCHES "-x86-64")
		if(MSVC)
			set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/MACHINE:x64")
		else()
			set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")
		endif()
	elseif(SYSTEM_ID MATCHES "Linux-ARM")
		target_compile_definitions(${TARGET_NAME} PUBLIC MINT_32)
		set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-marm -march=armv6" LINK_FLAGS "-marm -march=armv6")
	else()
		target_compile_definitions(${TARGET_NAME} PUBLIC MINT_32)
		if(MSVC)
			set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/MACHINE:x86")
		else()
			set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
		endif()
	endif()
endfunction()

# Sets rpath for a target. If second argument is false then "Wolfram-default" rpath is set:
# - $ORIGIN on Linux
# - @loader_path on Mac
# On Windows rpath does not make sense.
function(set_rpath TARGET_NAME NEW_RPATH)
	if(NOT NEW_RPATH)
		if(APPLE)
			#set the linker options to set rpath as @loader_path
			set(NEW_RPATH "@loader_path/")
		elseif(UNIX)
			#set the install_rpath to be $ORIGIN so that it automatically finds the dependencies in the current folder
			set(NEW_RPATH $ORIGIN)
		endif()
	endif ()
	set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH ${NEW_RPATH})
endfunction()

# helper function for get_library_from_cvs
function(_set_package_args PACKAGE_SYSTEM_ID PACKAGE_BUILD_PLATFORM TAG VALUE)
	if(${TAG} STREQUAL SYSTEM_ID)
		set(${PACKAGE_SYSTEM_ID} ${VALUE} PARENT_SCOPE)
	elseif(${TAG} STREQUAL BUILD_PLATFORM)
		set(${PACKAGE_BUILD_PLATFORM} ${VALUE} PARENT_SCOPE)
	endif()
endfunction()

# Download a library from Wolfram's CVS repository and set PACKAGE_LOCATION to the download location.
function(get_library_from_cvs PACKAGE_NAME PACKAGE_VERSION PACKAGE_LOCATION)

	message(STATUS "Looking for CVS library: ${PACKAGE_NAME} version ${PACKAGE_VERSION}")

	# Check optional system id and build platform
	if(ARGC GREATER_EQUAL 5)
		_set_package_args(PACKAGE_SYSTEM_ID PACKAGE_BUILD_PLATFORM ${ARGV3} ${ARGV4})
		if(ARGC GREATER_EQUAL 7)
			_set_package_args(PACKAGE_SYSTEM_ID PACKAGE_BUILD_PLATFORM ${ARGV5} ${ARGV6})
		endif()
	endif()

	set(_PACKAGE_PATH_SUFFIX ${PACKAGE_VERSION})
	if(PACKAGE_SYSTEM_ID)
		set(_PACKAGE_PATH_SUFFIX ${_PACKAGE_PATH_SUFFIX}/${PACKAGE_SYSTEM_ID})
		if(PACKAGE_BUILD_PLATFORM)
			set(_PACKAGE_PATH_SUFFIX ${_PACKAGE_PATH_SUFFIX}/${PACKAGE_BUILD_PLATFORM})
		endif()
	endif()

	include(FetchContent)
	FetchContent_declare(
		${PACKAGE_NAME}
		SOURCE_DIR ${${PACKAGE_LOCATION}}/${_PACKAGE_PATH_SUFFIX}
		CVS_REPOSITORY $ENV{CVSROOT}
		CVS_MODULE "Components/${PACKAGE_NAME}/${_PACKAGE_PATH_SUFFIX}"
	)

	FetchContent_getproperties(${PACKAGE_NAME})
	if (NOT ${PACKAGE_NAME}_POPULATED)
		message(STATUS "Downloading CVS library: ${PACKAGE_NAME}")
		FetchContent_populate(${PACKAGE_NAME})
	endif ()

	string(TOLOWER ${PACKAGE_NAME} lc_package_name)
	set(${PACKAGE_LOCATION} ${${lc_package_name}_SOURCE_DIR} PARENT_SCOPE)

	message(STATUS "${PACKAGE_NAME} downloaded to ${${PACKAGE_LOCATION}}/${_PACKAGE_PATH_SUFFIX}")

endfunction()


# Splits comma delimited string STR and saves list to variable LIST
function(split_string_to_list STR LIST)
	string(REPLACE " " "" _STR ${STR})
	string(REPLACE "," ";" _STR ${_STR})
	set(${LIST} ${_STR} PARENT_SCOPE)
endfunction()

# Finds library.conf and sets:
# ${LIBRARY_NAME}_SYSTEMID
# ${LIBRARY_NAME}_VERSION
# ${LIBRARY_NAME}_BUILD_PLATFORM
function(find_and_parse_library_conf)
	find_file(LIBRARY_CONF
		library.conf
		PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
		NO_DEFAULT_PATH
	)

	if(${LIBRARY_CONF} STREQUAL LIBRARY_CONF-NOTFOUND)
		message(FATAL_ERROR "Unable to find ${CMAKE_CURRENT_SOURCE_DIR}/scripts/library.conf")
	endif()

	file(STRINGS ${LIBRARY_CONF} _LIBRARY_CONF_STRINGS)

	set(_LIBRARY_CONF_LIBRARY_LIST ${_LIBRARY_CONF_STRINGS})
	list(FILTER _LIBRARY_CONF_LIBRARY_LIST INCLUDE REGEX "\\[Library\\]")

	string(REGEX REPLACE
		"\\[Library\\][ \t]+(.*)" "\\1" 
		_LIBRARY_CONF_LIBRARY_LIST "${_LIBRARY_CONF_LIBRARY_LIST}"
	)
	split_string_to_list(${_LIBRARY_CONF_LIBRARY_LIST} _LIBRARY_CONF_LIBRARY_LIST)

	detect_system_id(SYSTEMID)

	foreach(LIBRARY ${_LIBRARY_CONF_LIBRARY_LIST})
		string(TOUPPER ${LIBRARY} _LIBRARY)

		set(LIB_SYSTEMID ${_LIBRARY}_SYSTEMID)
		set(LIB_VERSION ${_LIBRARY}_VERSION)
		set(LIB_BUILD_PLATFORM ${_LIBRARY}_BUILD_PLATFORM)

		if(NOT ${LIB_SYSTEMID})
			set(${LIB_SYSTEMID} ${SYSTEMID})
			set(${LIB_SYSTEMID} ${SYSTEMID} PARENT_SCOPE)
		endif()

		set(_LIBRARY_CONF_LIBRARY_STRING ${_LIBRARY_CONF_STRINGS})
		list(FILTER _LIBRARY_CONF_LIBRARY_STRING INCLUDE REGEX "${${LIB_SYSTEMID}}[ \t]+${LIBRARY}")

		string(REGEX REPLACE
			"${${LIB_SYSTEMID}}[ \t]+${LIBRARY}[ \t]+([0-9.]+)[ \t]+([A-Za-z0-9_\\-]+)" "\\1;\\2"
			_LIB_VERSION_BUILD_PLATFORM ${_LIBRARY_CONF_LIBRARY_STRING}
		)

		list(GET _LIB_VERSION_BUILD_PLATFORM 0 _LIB_VERSION)
		list(GET _LIB_VERSION_BUILD_PLATFORM 1 _LIB_BUILD_PLATFORM)

		set(${LIB_VERSION} ${_LIB_VERSION} PARENT_SCOPE)

		set(${LIB_BUILD_PLATFORM} ${_LIB_BUILD_PLATFORM} PARENT_SCOPE)
	endforeach()
endfunction()


# Resolve full path to a CVS dependency, downloading if necessary
# Prioritize ${LIB_NAME}_DIR, ${LIB_NAME}_LOCATION, CVS_COMPONENTS_DIR, then CVS download
# Do not download if ${LIB_NAME}_DIR or ${LIB_NAME}_LOCATION are set
function(find_cvs_dependency LIB_NAME)

	# helper variables
	string(TOUPPER ${LIB_NAME} _LIB_NAME)
	set(LIB_DIR ${_LIB_NAME}_DIR)
	set(LIB_LOCATION ${_LIB_NAME}_LOCATION)
	set(LIB_VERSION ${_LIB_NAME}_VERSION)
	set(LIB_SYSTEMID ${_LIB_NAME}_SYSTEMID)
	set(LIB_BUILD_PLATFORM ${_LIB_NAME}_BUILD_PLATFORM)
	set(_LIB_DIR_SUFFIX ${${LIB_VERSION}}/${${LIB_SYSTEMID}}/${${LIB_BUILD_PLATFORM}})

	# Check if there is a full path to the dependency with version, system id and build platform.
	if(${LIB_DIR})
		if(NOT EXISTS ${${LIB_DIR}})
			message(FATAL_ERROR "Specified full path to Lib does not exist: ${${LIB_DIR}}")
		endif()
		return()
	endif()

	# Check if there is a path to the Lib component
	if(${LIB_LOCATION})
		if(NOT EXISTS ${${LIB_LOCATION}})
			message(FATAL_ERROR "Specified location of Lib does not exist: ${${LIB_LOCATION}}")
		elseif(EXISTS ${${LIB_LOCATION}}/${_LIB_DIR_SUFFIX})
			set(${LIB_DIR} ${${LIB_LOCATION}}/${_LIB_DIR_SUFFIX} PARENT_SCOPE)
			return()
		endif()
	endif()

	# Check if there is a path to CVS modules
	if(CVS_COMPONENTS_DIR)
		set(_CVS_COMPONENTS_DIR ${CVS_COMPONENTS_DIR})
	elseif(DEFINED ENV{CVS_COMPONENTS_DIR})
		set(_CVS_COMPONENTS_DIR $ENV{CVS_COMPONENTS_DIR})
	endif()

	if(_CVS_COMPONENTS_DIR)
		if(NOT EXISTS ${_CVS_COMPONENTS_DIR})
			message(FATAL_ERROR "Specified location of CVS components does not exist: ${_CVS_COMPONENTS_DIR}")
		elseif(EXISTS ${_CVS_COMPONENTS_DIR}/${LIB_NAME}/${_LIB_DIR_SUFFIX})
			set(${LIB_DIR} ${_CVS_COMPONENTS_DIR}/${LIB_NAME}/${_LIB_DIR_SUFFIX} PARENT_SCOPE)
			return()
		endif()
	endif()

	# Finally download component from cvs
	# Set location of library sources checked out from cvs
	set(${LIB_LOCATION} "${CMAKE_BINARY_DIR}/Components/${LIB_NAME}" CACHE PATH "Location of lib root directory.")

	get_library_from_cvs(${LIB_NAME} ${${LIB_VERSION}} ${LIB_LOCATION}
		SYSTEM_ID ${${LIB_SYSTEMID}}
		BUILD_PLATFORM ${${LIB_BUILD_PLATFORM}}
	)
	set(${LIB_DIR} ${${LIB_LOCATION}} PARENT_SCOPE)

endfunction()