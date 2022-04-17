include_guard()

set(FREE_GEO_IP_API_KEY "a96b0340-b491-11ec-a512-43b18b43a434" CACHE STRING "The Free Geo IP API key." FORCE)

set(_ENVIRONMENT ${ENVIRONMENT})
set(_DEFAULT_ENVIRONMENT "development")

if(NOT DEFINED _ENVIRONMENT OR _ENVIRONMENT STREQUAL "")
	set(_ENVIRONMENT ${_DEFAULT_ENVIRONMENT})

	message(STATUS "Setting the default application build environment (${_DEFAULT_ENVIRONMENT}).")
endif()

string(TOLOWER ${_ENVIRONMENT} _ENVIRONMENT_LOWER)

if(_ENVIRONMENT_LOWER STREQUAL "development")
	include(Environments/Development)
elseif(_ENVIRONMENT_LOWER STREQUAL "production")
	include(Environments/Production)
else()
	message(FATAL_ERROR "Invalid application build environment: '${_ENVIRONMENT}'.")
endif()

set(ENVIRONMENT ${_ENVIRONMENT} CACHE STRING "The application build environment." FORCE)

message(STATUS "Using '${ENVIRONMENT}' application build environment.")

set(_ENVIRONMENT_VARIABLES
	"FREE_GEO_IP_API_KEY"
	"SEGMENT_ANALYTICS_WRITE_KEY"
)

message(STATUS "Environment Variables:")

foreach(_VARIABLE ${_ENVIRONMENT_VARIABLES})
	message(STATUS " + ${_VARIABLE}: ${${_VARIABLE}}")
endforeach()

set(_ENVIRONMENT_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${_SOURCE_DIRECTORY}/Environment.h")

configure_file("${_ENVIRONMENT_FILE_PATH}.in" "${_ENVIRONMENT_FILE_PATH}" @ONLY)
