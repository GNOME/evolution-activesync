set(SOURCES
	libeastest.c
	libeastest.h
)

add_library(eastest STATIC ${SOURCES})

add_dependencies(eastest activesyncd)

target_compile_definitions(eastest PRIVATE
	-DG_LOG_DOMAIN=\"libeastest\"
)

target_compile_options(eastest PUBLIC
	${GLIB_CFLAGS}
	${GIO_CFLAGS}
	${EDATASERVER_CFLAGS}
)

target_include_directories(eastest PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/eas-daemon/libeas
	${CMAKE_BINARY_DIR}/eas-daemon/src
	${GLIB_INCLUDE_DIRS}
	${GIO_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
)

target_link_libraries(eastest
	easclient
	eas
	${GLIB_LDFLAGS}
	${GIO_LDFLAGS}
	${EDATASERVER_LDFLAGS}
)
