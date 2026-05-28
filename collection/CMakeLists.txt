set(SOURCES
	module-eas-backend.c
	e-eas-backend.c
	e-eas-backend.h
	e-eas-backend-factory.c
	e-eas-backend-factory.h
)

add_library(module-eas-backend MODULE ${SOURCES})

target_compile_options(module-eas-backend PRIVATE
	${EVOLUTION_MAIL_CFLAGS}
	${EDATASERVER_CFLAGS}
	${LIBEBACKEND_CFLAGS}
)

target_include_directories(module-eas-backend PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	${EVOLUTION_MAIL_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${LIBEBACKEND_INCLUDE_DIRS}
)

target_link_libraries(module-eas-backend
	${EVOLUTION_MAIL_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${LIBEBACKEND_LDFLAGS}
)

install(TARGETS module-eas-backend
	DESTINATION ${eds_moduledir}
)
