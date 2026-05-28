# Generate GDBus skeleton/proxy code from introspection XML
set(GENERATED_GDBUS_SYNC
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-sync.c
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-sync.h
)
gdbus_codegen(eas-sync.xml org.meego.activesyncd.Eas EasGDBus
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-sync GENERATED_GDBUS_SYNC)

set(GENERATED_GDBUS_MAIL
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-mail.c
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-mail.h
)
gdbus_codegen(eas-mail.xml org.meego.activesyncd.Eas EasGDBus
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-mail GENERATED_GDBUS_MAIL)

set(GENERATED_GDBUS_COMMON
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-common.c
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-common.h
)
gdbus_codegen(eas-common.xml org.meego.activesyncd.Eas EasGDBus
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-common GENERATED_GDBUS_COMMON)

set(GENERATED_GDBUS_TEST
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-test.c
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-test.h
)
gdbus_codegen(eas-test.xml org.meego.activesyncd.Eas EasGDBus
	${CMAKE_CURRENT_BINARY_DIR}/eas-gdbus-test GENERATED_GDBUS_TEST)

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
	${GENERATED_GDBUS_SYNC}
	${GENERATED_GDBUS_MAIL}
	${GENERATED_GDBUS_COMMON}
	${GENERATED_GDBUS_TEST}
)

add_executable(activesyncd ${SOURCES})

target_compile_definitions(activesyncd PRIVATE
	-DG_LOG_DOMAIN=\"activesyncd\"
	-DDISABLE_EAS_DAEMON
)

target_compile_options(activesyncd PRIVATE
	${GLIB_CFLAGS}
	${GIO_CFLAGS}
	${EDATASERVER_CFLAGS}
	${SOUP_CFLAGS}
)

target_include_directories(activesyncd PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/eas-daemon/libeas
	${CMAKE_SOURCE_DIR}/libeasclient
	${GLIB_INCLUDE_DIRS}
	${GIO_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${SOUP_INCLUDE_DIRS}
)

target_link_libraries(activesyncd
	eas
	easclient
	easaccount
	${GLIB_LDFLAGS}
	${GIO_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${SOUP_LDFLAGS}
)

install(TARGETS activesyncd
	DESTINATION ${LIBEXEC_INSTALL_DIR}
)
