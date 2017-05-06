cmake_minimum_required(VERSION 2.8.12)

set(NAME "pthreadVC2")
project(${NAME})

set(PTHREADS_SOURCES
	attr.c
	barrier.c
	cancel.c
	cleanup.c
	condvar.c
	create.c
	dll.c
	errno.c
	exit.c
	fork.c
	global.c
	misc.c
	mutex.c
	nonportable.c
	private.c
	rwlock.c
	sched.c
	semaphore.c
	signal.c
	spin.c
	sync.c
	tsd.c
)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_definitions(-DPTW32_BUILD -DHAVE_SIGSET_T -DHAVE_SIGNAL_H -DHAVE_MODE_T -DHAVE_STRUCT_TIMESPEC -DHAVE_SYS_TYPES_H)

add_library(${NAME} SHARED ${PTHREADS_SOURCES})

set(LIBRARY_DEPS
    ${WIN32_SYSTEM_LIBRARIES}
)

target_link_libraries(${NAME} ${LIBRARY_DEPS})
