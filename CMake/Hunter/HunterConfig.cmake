include_guard()

# wxWidgets Dependencies
hunter_config(CatchSelfTest
	URL "https://codeload.github.com/catchorg/catch2/zip/tags/v3.5.0"
	VERSION "3.5.0.0"
	SHA1 "fd37a71ebf9455181a44fe7e2a0d56bb3c07b973"
)

hunter_config(EXPAT
	URL "https://codeload.github.com/libexpat/libexpat/zip/tags/R_2_5_0"
	VERSION "2.5.0.0"
	SHA1 "67d6afffe9719484e9b1614c6759a4ca4281e2b6"
)

hunter_config(JPEG
	URL "https://codeload.github.com/libjpeg-turbo/libjpeg-turbo/zip/tags/3.0.1"
	VERSION "3.0.1.0"
	SHA1 "cd1ab159d6dccb7e14973dc4b72fe5f25a492fe2"
)

hunter_config(NanoSVG
	URL "https://github.com/Telefrag-Software/nanosvg/archive/refs/heads/hunter-2023-11-22.zip"
	VERSION "2023-11-22"
	SHA1 "69a5e869e95234e734f5200987cf6ce3811148ba"
)

hunter_config(PCRE2
	URL "https://codeload.github.com/Telefrag-Software/pcre/zip/tags/v10.43"
	VERSION "10.43.0"
	SHA1 "5601b530d91d74c4e2da79ade0e314087d413e6c"
)

hunter_config(png
	URL "https://github.com/Telefrag-Software/libpng/archive/refs/heads/hunter-1.6.40.0.zip"
	VERSION "1.6.40.0"
	SHA1 "caacb021be2661cf3d83f5892219fc98a910ec93"
	CMAKE_ARGS
		BUILD_SHARED_LIBS=OFF
		PNG_SHARED=OFF
		PNG_STATIC=ON
		PNG_EXECUTABLES=OFF
		PNG_TESTS=OFF
		PNG_DEBUG=OFF
		PNG_DISABLE_AWK=ON
)

# Application Dependencies
hunter_config(JDKSMIDI
	URL "https://github.com/Telefrag-Software/jdksmidi/archive/refs/heads/hunter-2014.08.11.zip"
	VERSION "2014.08.11"
	SHA1 "5c7724b326c7f9aa74991ac95e19b3ac1b7f8d94"
)

hunter_config(SndFile
	URL "https://codeload.github.com/libsndfile/libsndfile/zip/tags/1.2.2"
	VERSION "1.2.2"
	SHA1 "ac2f7d628dbd0c190deccba45670778ba4cab563"
	CMAKE_ARGS
		BUILD_TESTING=OFF
		INSTALL_MANPAGES=OFF
		ENABLE_CPACK=OFF
)

hunter_config(wxWidgets
	URL "https://github.com/Telefrag-Software/wxWidgets/archive/refs/heads/hunter-3.2.4.0.zip"
	VERSION "3.2.4.0"
	SHA1 "67d96df9a5b7f1d7bebeade3d34b5693bc97fc30"
	CMAKE_ARGS
		wxBUILD_MONOLITHIC=ON
		wxBUILD_SHARED=OFF
		wxUSE_STL=ON
		wxUSE_GUI=ON
)
