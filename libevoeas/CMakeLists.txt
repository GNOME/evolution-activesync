set(SOURCES
	camel-eas-settings.c
	camel-eas-settings.h
)

add_library(evoeas SHARED ${SOURCES})

set_target_properties(evoeas PROPERTIES
	OUTPUT_NAME evoeas
)

target_compile_definitions(evoeas PRIVATE
	-DG_LOG_DOMAIN=\"libevoeas\"
)

target_compile_options(evoeas PUBLIC
	${CAMEL_CFLAGS}
)

target_include_directories(evoeas PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CAMEL_INCLUDE_DIRS}
)

target_link_libraries(evoeas
	${CAMEL_LDFLAGS}
)

install(TARGETS evoeas
	DESTINATION ${EVOLUTION_privlibdir}
)
