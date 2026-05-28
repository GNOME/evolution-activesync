add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/eas-test-client-stub.h
	COMMAND ${DBUS_BINDING_TOOL}
		--prefix=eas_object
		--mode=glib-client
		${CMAKE_SOURCE_DIR}/eas-daemon/src/eas-test.xml
		> ${CMAKE_CURRENT_BINARY_DIR}/eas-test-client-stub.h
	DEPENDS ${CMAKE_SOURCE_DIR}/eas-daemon/src/eas-test.xml
	VERBATIM
)

set(SOURCES
	libeastest.c
	libeastest.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-test-client-stub.h
)

add_library(eastest STATIC ${SOURCES})

target_compile_definitions(eastest PRIVATE
	-DG_LOG_DOMAIN=\"libeastest\"
)

target_compile_options(eastest PUBLIC
	${GLIB_CFLAGS}
	${EDATASERVER_CFLAGS}
	${DBUS_GLIB_CFLAGS}
	${LIBXML_CFLAGS}
	${SOUP_CFLAGS}
)

target_include_directories(eastest PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/eas-daemon/libeas
	${GLIB_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${DBUS_GLIB_INCLUDE_DIRS}
	${LIBXML_INCLUDE_DIRS}
	${SOUP_INCLUDE_DIRS}
)

target_link_libraries(eastest
	easclient
	eas
	${GLIB_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${DBUS_GLIB_LDFLAGS}
	${LIBXML_LDFLAGS}
	${SOUP_LDFLAGS}
)
