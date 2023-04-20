cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(_SOURCE_DIRECTORY "Source")
set(_RESOURCES_DIRECTORY "Resources")
set(_LIBRARY_DIRECTORY "Libraries")
set(_PROJECT_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${_SOURCE_DIRECTORY}/Project.h")
set(APPLICATION_NAME "Duke Nukem 3D Mod Manager")
string(REPLACE " " "" _PROJECT_NAME ${APPLICATION_NAME})

add_subdirectory(${_LIBRARY_DIRECTORY}/Core)

list(APPEND CMAKE_MODULE_PATH "${CORE_DIRECTORY}/CMake")

include(Modules/GetGitCommitHash)
include(Hunter/HunterInit)

project(${_PROJECT_NAME} VERSION 0.0.1.0 LANGUAGES C CXX)

include(Environment)
include(ThirdPartyLibraries)

get_git_commit_hash()

set(APPLICATION_GIT_COMMIT_HASH ${GIT_COMMIT_HASH})

configure_file("${_PROJECT_FILE_PATH}.in" "${_PROJECT_FILE_PATH}" @ONLY)

if(WIN32)
	set(_RESOURCE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Windows/Resource.rc")
	set(AUTHOR_NAME "Kevin Scroggins")
	string(TIMESTAMP CURRENT_YEAR "%Y")
	string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
	configure_file("${_RESOURCE_FILE_PATH}.in" "${_RESOURCE_FILE_PATH}" @ONLY)
endif()

include(SourceFiles)
include(ResourceFiles)

if(MSVC)
	add_compile_options(/bigobj)
else()
	add_compile_options(-Wa,-mbig-obj)
endif()

add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${_SOURCE_DIRECTORY}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Core
		${wxWidgets_LIBRARIES}
)
