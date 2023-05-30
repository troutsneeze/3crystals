project("3crystals")
cmake_minimum_required(VERSION 2.8.12)

# Options

option(DEBUG "Debug build" OFF)
option(USE_STATIC_LIBS "Link with static libraries" ON)
option(STEAMWORKS "Build for Steam" OFF)

set(USER_INCLUDE_PATH "" CACHE FILEPATH "Any extra include directories you want")
set(USER_LIBRARY_PATH "" CACHE FILEPATH "Any extra library directories you want")

include_directories("include")
include_directories(${USER_INCLUDE_PATH})
link_directories(${USER_LIBRARY_PATH})

set(SHIM4_LIB shim4)
set(TGUI6_LIB tgui6)

# Inputs

set(SOURCES
	src/general.cpp
	src/gui.cpp
	src/main.cpp
	src/widgets.cpp
)

if(WIN32)
	list(APPEND SOURCES 3crystals.manifest versioninfo.rc windows_icons.rc)
endif()

# Outputs

add_executable("3crystals" ${SOURCES})

# Platform specifics

if(MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fstack-protector") # because of how FLAC is compiled. This requires ssp DLL
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows")
	SET(FLAGS "${FLAGS} -DUSE_TTF")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1")
	endif()
	if(USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} -DTGUI6_STATIC -DSHIM4_STATIC")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api64)
	endif()
	#set(FLAGS "${FLAGS} /DUSE_TTF")
	set_target_properties("3crystals" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("3crystals" PROPERTIES OUTPUT_NAME "3 Crystals")
	set(LIBS ${LIBS} ${SHIM4_LIB} ${TGUI6_LIB} mingw32 SDL2main SDL2 FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi version)
elseif(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /IGNORE:4075 /IGNORE:4099")
	SET(FLAGS "${FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_USE_MATH_DEFINES /EHsc /MP /W3 /wd4244 /wd4800 /wd4101 /MT")
	SET(FLAGS "${FLAGS} /DNTDDI_VERSION=0x06010000 /D_WIN32_WINNT=0x601")
	SET(FLAGS "${FLAGS} /DUSE_TTF")
	if(DEBUG)
		set(FLAGS "${FLAGS} /DDEBUG=1")
	endif()
	if(USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} /DTGUI6_STATIC /DSHIM4_STATIC")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} /DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api64)
	endif()
	#set(FLAGS "${FLAGS} /DUSE_TTF")
	set_target_properties("3crystals" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("3crystals" PROPERTIES OUTPUT_NAME "3 Crystals")
	set(LIBS ${LIBS} ${SHIM4_LIB} ${TGUI6_LIB} SDL2 SDL2main libpng16_static libvorbis_static ogg FLAC freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt)
elseif(APPLE)
	SET(FLAGS "-Wall -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-overloaded-virtual -DTGUI6_STATIC -DSHIM4_STATIC")
	set(FLAGS "${FLAGS} -stdlib=libc++ -std=c++11")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api)
	endif()
	if(DEMO)
		set(FLAGS "${FLAGS} -DDEMO")
	endif()
	set_target_properties("3crystals" PROPERTIES BUILD_WITH_INSTALL_RPATH on)
	set_target_properties("3crystals" PROPERTIES INSTALL_RPATH "@executable_path/")
	set_target_properties("3crystals" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("3crystals" PROPERTIES LINK_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework CoreAudio -framework AudioUnit -framework ForceFeedback -framework Carbon")
	set_target_properties("3crystals" PROPERTIES OUTPUT_NAME "3 Crystals")
	#set(LIBS ${LIBS} ${SHIM4_LIB} ${TGUI6_LIB} SDL2 vorbisfile vorbis ogg z)
	set(LIBS ${LIBS} ${SHIM4_LIB} ${TGUI6_LIB} SDL2 z)
else() # Linux
	SET(FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -std=c++0x")
	set(LFLAGS "-Wl,-rpath,\$ORIGIN")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g -fsanitize=address -fsanitize=leak")
		set(LFLAGS "${LFLAGS} -fsanitize=address -fsanitize=leak")
	else()
		set(FLAGS "${FLAGS}")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api)
	endif()
	if(DEMO)
		set(FLAGS "${FLAGS} -DDEMO")
	endif()
	set(FLAGS "${FLAGS} -DUSE_TTF")
	set_target_properties("3crystals" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("3crystals" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("3crystals" PROPERTIES OUTPUT_NAME "3 Crystals")
	set(LIBS ${LIBS} ${SHIM4_LIB} ${TGUI6_LIB} SDL2 GL Xcursor X11 FLAC vorbis ogg freetype png z)
endif()

target_link_libraries("3crystals" ${LIBS})
