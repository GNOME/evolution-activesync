add_pkgconfig_file(libeasaccount.pc.in libeasaccount.pc)

set(SOURCES
	eas-account.c
	eas-account.h
	eas-account-list.c
	eas-account-list.h
)

add_library(easaccount SHARED ${SOURCES})

set_target_properties(easaccount PROPERTIES
	VERSION "${LIBEASACCOUNT_CURRENT}.${LIBEASACCOUNT_REVISION}.${LIBEASACCOUNT_AGE}"
	SOVERSION ${LIBEASACCOUNT_CURRENT}
	OUTPUT_NAME easaccount
)

target_compile_definitions(easaccount PRIVATE
	-DG_LOG_DOMAIN=\"libeasaccount\"
)

target_compile_options(easaccount PUBLIC
	${GLIB_CFLAGS}
	${GIO_CFLAGS}
	${EDATASERVER_CFLAGS}
)

target_include_directories(easaccount PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${GLIB_INCLUDE_DIRS}
	${GIO_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
)

target_link_libraries(easaccount
	${GLIB_LDFLAGS}
	${GIO_LDFLAGS}
	${EDATASERVER_LDFLAGS}
)

install(TARGETS easaccount
	DESTINATION ${LIB_INSTALL_DIR}
)

install(FILES eas-account.h eas-account-list.h
	DESTINATION ${privincludedir}/eas-account
)
