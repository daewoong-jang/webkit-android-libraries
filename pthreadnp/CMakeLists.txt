cmake_minimum_required(VERSION 2.8.12)

set(NAME "pthreadnp")
project(${NAME})

set(PTHREADNP_SOURCES
    pthread.cpp
)

add_definitions(-DBUILDING_PTHREADNP)

if (WIN32)
    add_subdirectory(pthreads-win32)

    add_definitions(-DHAVE_SIGNAL_H)

    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/android/system/core/include"
    )

    add_library(${NAME} SHARED ${PTHREADNP_SOURCES})

    set(LIBRARY_DEPS
        cutils
        ndk
        pthreadVC2
        ${WIN32_SYSTEM_LIBRARIES}
    )

else ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    add_library(${NAME} STATIC ${PTHREADNP_SOURCES})

endif()

#link the module like any other
target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(pthreadnp)

if (WIN32)
    copy_library_headers_directory(pthreadnp . include)
else ()
    copy_library_headers(pthreadnp pthread.h include)
endif ()
