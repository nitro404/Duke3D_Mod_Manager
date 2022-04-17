cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(_SOURCE_DIRECTORY "Source")
set(_RESOURCES_DIRECTORY "Resources")
set(_LIBRARY_DIRECTORY "Libraries")
set(_PROJECT_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${_SOURCE_DIRECTORY}/Project.h")

add_subdirectory(${_LIBRARY_DIRECTORY}/Core)

list(APPEND CMAKE_MODULE_PATH "${CORE_DIRECTORY}/CMake")

include(Modules/GetGitCommitHash)
include(Hunter/HunterInit)

project(DukeNukem3DModManager VERSION 0.0.1 LANGUAGES C CXX)

include(Environment)
include(ThirdPartyLibraries)

get_git_commit_hash()

set(APPLICATION_GIT_COMMIT_HASH ${GIT_COMMIT_HASH})

configure_file("${_PROJECT_FILE_PATH}.in" "${_PROJECT_FILE_PATH}" @ONLY)

include(SourceFiles)

list(APPEND SOURCE_FILES ${SOURCE_FILES_${PLATFORM_UPPER}})

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

include(ResourceFiles)

list(APPEND RESOURCE_FILES ${RESOURCE_FILES_${PLATFORM_UPPER}})

list(TRANSFORM RESOURCE_FILES PREPEND "${_RESOURCES_DIRECTORY}/")

list(APPEND SOURCE_FILES ${RESOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${_SOURCE_DIRECTORY}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Core
)
