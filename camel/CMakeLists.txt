set(SOURCES
	camel-eas-folder.c
	camel-eas-folder.h
	camel-eas-message-info.c
	camel-eas-message-info.h
	camel-eas-private.h
	camel-eas-provider.c
	camel-eas-store.c
	camel-eas-store.h
	camel-eas-store-summary.c
	camel-eas-store-summary.h
	camel-eas-summary.c
	camel-eas-summary.h
	camel-eas-transport.c
	camel-eas-transport.h
	camel-eas-utils.c
	camel-eas-utils.h
)

add_library(cameleas MODULE ${SOURCES})

target_compile_definitions(cameleas PRIVATE
	-DG_LOG_DOMAIN=\"camel-eas-provider\"
)

target_compile_options(cameleas PRIVATE
	${CAMEL_CFLAGS}
	${EDATASERVER_CFLAGS}
	${LIBECAL_CFLAGS}
	${SOUP_CFLAGS}
	${LIBEBACKEND_CFLAGS}
	${EVOLUTION_MAIL_CFLAGS}
	${DBUS_GLIB_CFLAGS}
)

target_include_directories(cameleas PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/libeasclient
	${CMAKE_SOURCE_DIR}/libevoeas
	${CAMEL_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${SOUP_INCLUDE_DIRS}
	${LIBEBACKEND_INCLUDE_DIRS}
	${EVOLUTION_MAIL_INCLUDE_DIRS}
	${DBUS_GLIB_INCLUDE_DIRS}
)

target_link_libraries(cameleas
	easclient
	evoeas
	${CAMEL_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${LIBEBACKEND_LDFLAGS}
	${SOUP_LDFLAGS}
	${EVOLUTION_MAIL_LDFLAGS}
	${DBUS_GLIB_LDFLAGS}
)

install(TARGETS cameleas
	DESTINATION ${camel_providerdir}
)

install(FILES libcameleas.urls
	DESTINATION ${camel_providerdir}
)
