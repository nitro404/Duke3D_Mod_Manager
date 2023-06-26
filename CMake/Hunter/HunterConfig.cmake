include_guard()

# wxWidgets Dependencies
hunter_config(CatchSelfTest
	URL "https://github.com/Telefrag-Software/Catch2/archive/refs/heads/hunter-wx-5f5e4cec.zip"
	VERSION "5f5e4cec"
	SHA1 "178f730fe08c8d8de525790a17fc5fe6f9595181"
)

hunter_config(EXPAT
	URL "https://github.com/Telefrag-Software/libexpat/archive/refs/heads/hunter-wx-1187e407.zip"
	VERSION "1187e407"
	SHA1 "e7ff5d035356cd87bed5f7613bc5d1f69b9b5cb8"
)

hunter_config(JPEG
	URL "https://github.com/Telefrag-Software/libjpeg-turbo/archive/refs/heads/hunter-wx-8524936.zip"
	VERSION "8524936"
	SHA1 "b135f44ecafa08c91cd60ac5433e6d4fa5dfc990"
)

hunter_config(nanosvg
	URL "https://github.com/Telefrag-Software/nanosvg/archive/refs/heads/hunter-wx-26db6fe.zip"
	VERSION "26db6fe"
	SHA1 "7e882edb4ffa7800c7944e91c8ef00c628d70e89"
)

hunter_config(PCRE2
	URL "https://github.com/Telefrag-Software/pcre/archive/refs/heads/hunter-wx-5b934c2.zip"
	VERSION "5b934c2"
	SHA1 "2833e75f036154c6b900313e24c4982b9b70b026"
)

hunter_config(png
	URL "https://github.com/Telefrag-Software/libpng/archive/refs/heads/hunter-1.6.39.0.zip"
	VERSION "hunter-1.6.39.0"
	SHA1 "6c450efdd05ff38c5cebff40147dd95d2787b365"
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

hunter_config(wxWidgets
	URL "https://github.com/Telefrag-Software/wxWidgets/archive/refs/heads/hunter-3.2.1.0.zip"
	VERSION "3.2.1.0"
	SHA1 "f3aac0c796fdab697979e5b45b31e8db3d4dd10b"
	CMAKE_ARGS
		wxBUILD_MONOLITHIC=ON
		wxBUILD_SHARED=OFF
		wxUSE_STL=ON
		wxUSE_GUI=ON
		wxUSE_LIBLZMA=OFF
)
