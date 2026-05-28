set(SOURCES
	module-eas-mail-config.c
	e-mail-config-eas-backend.c
	e-mail-config-eas-backend.h
)

add_library(module-eas-mail-config MODULE ${SOURCES})

target_compile_options(module-eas-mail-config PRIVATE
	${GIO_CFLAGS}
	${EVOLUTION_MAIL_CFLAGS}
	${EDATASERVER_CFLAGS}
	${LIBEBACKEND_CFLAGS}
)

target_include_directories(module-eas-mail-config PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/libeasclient
	${GIO_INCLUDE_DIRS}
	${EVOLUTION_MAIL_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${LIBEBACKEND_INCLUDE_DIRS}
)

target_link_libraries(module-eas-mail-config
	easclient
	evoeas
	${GIO_LDFLAGS}
	${EVOLUTION_MAIL_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${LIBEBACKEND_LDFLAGS}
)

install(TARGETS module-eas-mail-config
	DESTINATION ${evo_moduledir}
)
