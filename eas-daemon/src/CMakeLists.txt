# Generate D-Bus server stubs
find_program(DBUS_BINDING_TOOL dbus-binding-tool REQUIRED)

foreach(_stub sync common mail test)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/eas-${_stub}-stub.h
		COMMAND ${DBUS_BINDING_TOOL}
			--prefix=eas_${_stub}
			--mode=glib-server
			${CMAKE_CURRENT_SOURCE_DIR}/eas-${_stub}.xml
			> ${CMAKE_CURRENT_BINARY_DIR}/eas-${_stub}-stub.h
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eas-${_stub}.xml
		VERBATIM
	)
endforeach()

# Generate marshal files
glib_genmarshal(eas-marshal eas_marshal eas-marshal.list)

set(SOURCES
	activesyncd-common-defs.h
	activesyncd-server.c
	eas-common.c
	eas-common.h
	eas-interface-base.c
	eas-interface-base.h
	eas-mail.c
	eas-mail.h
	eas-sync.c
	eas-sync.h
	eas-test.c
	eas-test.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-marshal.c
	${CMAKE_CURRENT_BINARY_DIR}/eas-marshal.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-sync-stub.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-common-stub.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-mail-stub.h
	${CMAKE_CURRENT_BINARY_DIR}/eas-test-stub.h
)

add_executable(activesyncd ${SOURCES})

target_compile_definitions(activesyncd PRIVATE
	-DG_LOG_DOMAIN=\"activesyncd\"
	-DDISABLE_EAS_DAEMON
)

target_compile_options(activesyncd PRIVATE
	${GLIB_CFLAGS}
	${EDATASERVER_CFLAGS}
	${DBUS_GLIB_CFLAGS}
	${SOUP_CFLAGS}
)

target_include_directories(activesyncd PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/eas-daemon/libeas
	${CMAKE_SOURCE_DIR}/libeasclient
	${GLIB_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${DBUS_GLIB_INCLUDE_DIRS}
	${SOUP_INCLUDE_DIRS}
)

target_link_libraries(activesyncd
	eas
	easclient
	easaccount
	${GLIB_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${DBUS_GLIB_LDFLAGS}
	${SOUP_LDFLAGS}
)

install(TARGETS activesyncd
	DESTINATION ${LIBEXEC_INSTALL_DIR}
)
