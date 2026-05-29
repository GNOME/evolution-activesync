add_pkgconfig_file(libeasclient.pc.in libeasclient.pc)

set(SOURCES
	eas-attachment.c
	eas-attachment.h
	eas-dbus-client.c
	eas-dbus-client.h
	eas-email-info.c
	eas-email-info.h
	eas-errors.c
	eas-errors.h
	eas-folder.c
	eas-folder.h
	eas-item-info.c
	eas-item-info.h
	eas-logger.c
	eas-logger.h
	eas-mail-errors.h
	eas-provision-list.c
	eas-provision-list.h
	eas-sync-errors.h
	libeasmail.c
	libeasmail.h
	libeassync.c
	libeassync.h
)

add_library(easclient SHARED ${SOURCES})

set_target_properties(easclient PROPERTIES
	VERSION "${LIBEASCLIENT_CURRENT}.${LIBEASCLIENT_REVISION}.${LIBEASCLIENT_AGE}"
	SOVERSION ${LIBEASCLIENT_CURRENT}
	OUTPUT_NAME easclient
)

target_compile_definitions(easclient PRIVATE
	-DG_LOG_DOMAIN=\"libeasclient\"
)

target_compile_options(easclient PUBLIC
	${GIO_CFLAGS}
	${LIBXML_CFLAGS}
)

target_include_directories(easclient PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${GIO_INCLUDE_DIRS}
	${LIBXML_INCLUDE_DIRS}
)

target_link_libraries(easclient
	${GIO_LDFLAGS}
	${LIBXML_LDFLAGS}
)

install(TARGETS easclient
	DESTINATION ${LIB_INSTALL_DIR}
)

install(FILES
	eas-attachment.h
	eas-email-info.h
	eas-errors.h
	eas-folder.h
	eas-item-info.h
	eas-mail-errors.h
	eas-sync-errors.h
	libeasmail.h
	libeassync.h
	DESTINATION ${privincludedir}/eas-client
)
