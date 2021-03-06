cmake_minimum_required(VERSION 3.5)

project(webkit-android-libraries)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
define_property(TARGET PROPERTY FOLDER INHERITED BRIEF_DOCS "folder" FULL_DOCS "IDE folder name")

if (DEFINED ENV{WEBKIT_LIBRARIES})
    set(WEBKIT_LIBRARIES_DIR "$ENV{WEBKIT_LIBRARIES}")
else ()
    set(WEBKIT_LIBRARIES_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Path to the directory where products will be copied to.")
endif ()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${WEBKIT_LIBRARIES_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${WEBKIT_LIBRARIES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WEBKIT_LIBRARIES_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(HelperMacros)
include(Options${CMAKE_SYSTEM_NAME})

if (WIN32)
    set(CMAKE_ASM_NASM_COMPILER vsyasm)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB")
    set(WIN32_SYSTEM_LIBRARIES concrt libcmt libcpmt msvcrt OLDNAMES Version vcruntime ucrt Shlwapi)

    add_definitions(-D_WINSOCKAPI_ -D__STDC_FORMAT_MACROS -DHAVE_STRUCT_TIMESPEC)
    add_definitions(-DCAIRO_WIN32_STATIC_BUILD -DCURL_STATICLIB -DLIBXML_STATIC -DLIBXSLT_STATIC)
    add_compile_options(/wd4668)

    include_directories("${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/android-14/arch-x86/usr/include")

    macro(ADD_EXECUTABLE _name)
        _add_executable(${ARGV})
        if (TARGET ${_name})
            target_link_libraries(${_name} ndk ${WIN32_SYSTEM_LIBRARIES})
        endif()
    endmacro()
endif ()

# Common projects.
add_subdirectory(android/prebuilts/ndk/current/sources/cxx-stl/gnu-libstdc++)
add_subdirectory(android/system/core/libcutils)
add_subdirectory(boringssl)
add_subdirectory(cairo)
add_subdirectory(curl)
add_subdirectory(fontconfig)
add_subdirectory(giflib)
add_subdirectory(hyphenation)
add_subdirectory(icu)
add_subdirectory(libjpeg-turbo)
add_subdirectory(libpng)
add_subdirectory(libxml2)
add_subdirectory(libxslt)
add_subdirectory(pixman)
add_subdirectory(safeint)
add_subdirectory(sqlite)
add_subdirectory(webp)

add_dependencies(libcurl ssl crypto)
add_dependencies(icuin icuuc)
add_dependencies(icuuc icudt)
add_dependencies(xml2 icuin)

if (WIN32) # Windows.
    add_subdirectory(android/prebuilts/ndk)
    add_subdirectory(android/system/core/liblog)
    add_subdirectory(jsoncpp)
    add_subdirectory(libdl)
    add_subdirectory(wtl)
    add_subdirectory(zlib)

    add_dependencies(ndk log cutils)
    add_dependencies(ssl ndk)
    add_dependencies(z ndk)
else () # Android.
    add_subdirectory(freetype)
    add_subdirectory(harfbuzz)
endif ()
