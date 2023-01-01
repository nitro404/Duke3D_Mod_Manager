cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

option(GUI_ENABLED "Enables the grapical user interface." ON)

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

project(${_PROJECT_NAME} VERSION 0.0.1 LANGUAGES C CXX)

include(Environment)
include(ThirdPartyLibraries)

get_git_commit_hash()

set(APPLICATION_GIT_COMMIT_HASH ${GIT_COMMIT_HASH})

configure_file("${_PROJECT_FILE_PATH}.in" "${_PROJECT_FILE_PATH}" @ONLY)

include(SourceFiles)
include(ResourceFiles)

if(GUI_ENABLED)
	set(_EXECUTABLE_TYPE WIN32)
	set(_GUI_LIBRARIES ${wxWidgets_LIBRARIES})
endif()

add_executable(${PROJECT_NAME} ${_EXECUTABLE_TYPE} ${ALL_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${_SOURCE_DIRECTORY}
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Core
		${_GUI_LIBRARIES}
)
