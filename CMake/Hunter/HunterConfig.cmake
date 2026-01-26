include_guard()

# CatchSelfTest is used by wxWidgets.
hunter_config(CatchSelfTest
	URL "https://codeload.github.com/catchorg/catch2/zip/tags/v3.5.0"
	VERSION "3.5.0.0"
	SHA1 "fd37a71ebf9455181a44fe7e2a0d56bb3c07b973"
	CMAKE_ARGS
		CATCH_INSTALL_DOCS=OFF
		CATCH_INSTALL_EXTRAS=OFF
		CATCH_DEVELOPMENT_BUILD=OFF
		CATCH_ENABLE_REPRODUCIBLE_BUILD=ON
		CATCH_BUILD_TESTING=OFF
		CATCH_BUILD_EXAMPLES=OFF
		CATCH_BUILD_EXTRA_TESTS=OFF
		CATCH_BUILD_FUZZERS=OFF
		CATCH_BUILD_BENCHMARKS=OFF
		CATCH_ENABLE_COVERAGE=OFF
		CATCH_ENABLE_WERROR=ON
		CATCH_BUILD_SURROGATES=OFF
		CATCH_ENABLE_CONFIGURE_TESTS=OFF
		CATCH_ENABLE_CMAKE_HELPER_TESTS=OFF
)

# LibEXPAT is used by wxWidgets.
hunter_config(Expat
	URL "https://codeload.github.com/libexpat/libexpat/zip/tags/R_2_7_3"
	VERSION "2.7.3.0"
	SHA1 "af45665222bb5c749a0750e971f2f72839f8999c"
	CMAKE_ARGS
		EXPAT_BUILD_TOOLS=OFF
		EXPAT_BUILD_EXAMPLES=OFF
		EXPAT_BUILD_TESTS=OFF
		EXPAT_SHARED_LIBS=OFF
		EXPAT_BUILD_DOCS=OFF
		EXPAT_BUILD_FUZZERS=OFF
		EXPAT_BUILD_PKGCONFIG=ON
		EXPAT_OSSFUZZ_BUILD=OFF
		EXPAT_ENABLE_INSTALL=ON
		EXPAT_DTD=ON
		EXPAT_GE=ON
		EXPAT_NS=ON
		EXPAT_WARNINGS_AS_ERRORS=OFF
		EXPAT_ATTR_INFO=OFF
		EXPAT_LARGE_SIZE=OFF
		EXPAT_MIN_SIZE=OFF
		EXPAT_MSVC_STATIC_CRT=ON
)

# LibJPEG is used by LibTIFF and wxWidgets.
hunter_config(Jpeg
	URL "https://codeload.github.com/Telefrag-Software/libjpeg/zip/tags/9f"
	VERSION "9f"
	SHA1 "983e8b6e4a508944522f48c2dd680331012a322f"
)

# Lexilla is used by wxWidgets.
hunter_config(lexilla
	URL "https://github.com/Telefrag-Software/lexilla/archive/refs/heads/hunter-wx-5.4.4.1.zip"
	VERSION "5.4.4.1"
	SHA1 "7f44fe1fc6148a2dc495d549b8f32590af23d056"
)

# Scintilla is used by Lexilla and wxWidgets.
hunter_config(scintilla
	URL "https://github.com/Telefrag-Software/scintilla/archive/refs/heads/hunter-wx-5.0.0.1.zip"
	VERSION "5.0.0.1"
	SHA1 "d390c96a0c02e879732e5c41e4783ede438f2952"
)

# LibTIFF is used by wxWidgets.
hunter_config(TIFF
	URL "https://github.com/Telefrag-Software/libtiff/archive/refs/heads/hunter-4.7.1.0.zip"
	VERSION "4.7.1.0"
	SHA1 "761df43499ab065772209b5da0a228056d11a4ee"
	CMAKE_ARGS
		BUILD_SHARED_LIBS=OFF
		tiff-cxx=ON
		tiff-opengl=OFF
		extra-warnings=OFF
		fatal-warnings=OFF
		strip-chopping=ON
		defer-strile-load=OFF
		chunky-strip-read=OFF
		extrasample-as-alpha=ON
		check-ycbcr-subsampling=ON
		sphinx=OFF
		jbig=OFF
		jpeg=ON
		lerc=OFF
		libdeflate=OFF
		lzma=ON
		pixarlog=ON
		webp=ON
		zlib=ON
		zstd=ON
)

# NanoSVG is used by wxWidgets.
hunter_config(nanosvg
	URL "https://github.com/Telefrag-Software/nanosvg/archive/refs/heads/hunter-2025-11-21.zip"
	VERSION "2025.11.21"
	SHA1 "a5e35c0ad9a3784b0627b84ce03c98de211d112d"
)

# PCRE2 is used by wxWidgets.
hunter_config(pcre2
	URL "https://github.com/Telefrag-Software/pcre/archive/refs/heads/hunter-10.43.0.1.zip"
	VERSION "10.43.0.1"
	SHA1 "b6a4700010ff2c97239323842298b2efb8d2baac"
	CMAKE_ARGS
		BUILD_STATIC_LIBS=ON
		PCRE2_BUILD_PCRE2_8=ON
		PCRE2_BUILD_PCRE2_16=OFF
		PCRE2_BUILD_PCRE2_32=OFF
		PCRE2_STATIC_PIC=OFF
		PCRE2_REBUILD_CHARTABLES=OFF
		PCRE2_SHOW_REPORT=ON
		PCRE2_BUILD_PCRE2GREP=OFF
		PCRE2_BUILD_TESTS=OFF
		NON_STANDARD_LIB_PREFIX=OFF
		NON_STANDARD_LIB_SUFFIX=OFF
		INSTALL_MSVC_PDB=ON
		PCRE2_SYMVERS=ON
		PCRE2_SUPPORT_LIBBZ2=ON
		PCRE2_SUPPORT_LIBZ=ON
		PCRE2_SUPPORT_LIBEDIT=OFF
		PCRE2_SUPPORT_LIBREADLINE=OFF
		PCRE2_SUPPORT_JIT=OFF
)

# LibPNG is used by wxWidgets.
hunter_config(PNG
	URL "https://github.com/Telefrag-Software/libpng/archive/refs/heads/hunter-1.6.53.2.zip"
	VERSION "1.6.53.2"
	SHA1 "caa094e0d01bc75599c8574ebb0286945a4139c4"
	CMAKE_ARGS
		BUILD_SHARED_LIBS=OFF
		PNG_SHARED=OFF
		PNG_STATIC=ON
		PNG_EXECUTABLES=OFF
		PNG_TESTS=OFF
		PNG_DEBUG=OFF
		PNG_DISABLE_AWK=ON
		SKIP_INSTALL_ALL=OFF
		SKIP_INSTALL_HEADERS=OFF
		SKIP_INSTALL_LIBRARIES=OFF
		SKIP_INSTALL_EXECUTABLES=ON
		SKIP_INSTALL_PROGRAMS=ON
		SKIP_INSTALL_EXPORT=OFF
		SKIP_INSTALL_CONFIG_FILE=OFF
)

hunter_config(JDKSMIDI
	URL "https://github.com/Telefrag-Software/jdksmidi/archive/refs/heads/hunter-2014.08.11a.zip"
	VERSION "2014.08.11a"
	SHA1 "2f47cc9b8eb1ef540aa5631501d88231c7b3b8ab"
)

hunter_config(SndFile
	URL "https://codeload.github.com/Telefrag-Software/libsndfile/zip/tags/1.2.2.1"
	VERSION "1.2.2.1"
	SHA1 "92258be67894f09f55f91cfa0d19e0957271a9dc"
	CMAKE_ARGS
		BUILD_TESTING=OFF
		INSTALL_MANPAGES=OFF
		ENABLE_CPACK=OFF
)

# LibWebP is used by LibTIFF and wxWidgets.
hunter_config(WebP
	URL "https://github.com/Telefrag-Software/libwebp/archive/refs/heads/hunter-1.6.0.0.zip"
	VERSION "1.6.0.0"
	SHA1 "ce4b8488d82999dee486a595e2f1e1751958def4"
	CMAKE_ARGS
		WEBP_BUILD_SHARED_LIBS=OFF
		WEBP_LINK_STATIC=ON
		WEBP_BUILD_ANIM_UTILS=OFF
		WEBP_BUILD_CWEBP=OFF
		WEBP_BUILD_DWEBP=OFF
		WEBP_BUILD_GIF2WEBP=OFF
		WEBP_BUILD_IMG2WEBP=OFF
		WEBP_BUILD_VWEBP=OFF
		WEBP_BUILD_WEBPINFO=OFF
		WEBP_BUILD_LIBWEBPMUX=ON
		WEBP_BUILD_WEBPMUX=OFF
		WEBP_BUILD_EXTRAS=OFF
		WEBP_BUILD_WEBP_JS=OFF
		WEBP_BUILD_FUZZTEST=OFF
		WEBP_USE_THREAD=ON
		WEBP_NEAR_LOSSLESS=ON
		WEBP_ENABLE_SWAP_16BIT_CSP=OFF
		WEBP_ENABLE_WUNUSED_RESULT=OFF
		WEBP_UNICODE=ON
)

hunter_config(wxWidgets
	URL "https://github.com/Telefrag-Software/wxWidgets/archive/refs/heads/hunter-3.3.1.0.zip"
	VERSION "3.3.1.0"
	SHA1 "97c1225e7fa387831d093eb88a9de6ea1d8eba52"
	CMAKE_ARGS
		wxBUILD_MONOLITHIC=ON
		wxBUILD_SHARED=OFF
		wxUSE_GUI=ON
		wxBUILD_SAMPLES=OFF
		wxBUILD_DEMOS=OFF
		wxBUILD_BENCHMARKS=OFF
		wxBUILD_PIC=ON
		wxUSE_NO_RTTI=OFF
		wxBUILD_INSTALL_PDB=ON
)
