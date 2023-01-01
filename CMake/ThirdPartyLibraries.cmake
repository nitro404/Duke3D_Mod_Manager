include_guard()

if(GUI_ENABLED)
	# wxWidgets Dependencies
	hunter_add_package(EXPAT)
	hunter_add_package(JPEG)
	hunter_add_package(nanosvg)
	hunter_add_package(PCRE2)
	hunter_add_package(png)

	find_package(EXPAT CONFIG REQUIRED)
	find_package(JPEG CONFIG REQUIRED)
	find_package(nanosvg CONFIG REQUIRED)
	find_package(PCRE2 CONFIG REQUIRED)
	find_package(png CONFIG REQUIRED)
	find_package(OpenGL)

	# Application Dependencies
	hunter_add_package(wxWidgets)

	find_package(wxWidgets CONFIG REQUIRED)
endif()
