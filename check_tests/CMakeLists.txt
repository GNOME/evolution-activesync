set(SOURCES
	eas_autodiscover_tests.c
	eas_cal_info_translator_test.c
	eas_con_info_translator_test.c
	eas_connection_tests.c
	eas_daemon_tests.c
	eas_email_info_translator_test.c
	eas_folderhierarchy_tests.c
	eas_gobjects_unittests.c
	eas_libeascal_tests.c
	eas_libeascon_tests.c
	eas_libeasmail_tests.c
	eas_libeassync_tests.c
	eas_main_tests.c
)

add_executable(check_eas EXCLUDE_FROM_ALL ${SOURCES})

target_compile_options(check_eas PRIVATE
	${GLIB_CFLAGS}
	${DBUS_GLIB_CFLAGS}
	${EDATASERVER_CFLAGS}
	${CHECK_CFLAGS}
)

target_include_directories(check_eas PRIVATE
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/libeasclient
	${CMAKE_SOURCE_DIR}/libeasaccount/src
	${CMAKE_SOURCE_DIR}/eas-daemon/libeas
	${CMAKE_SOURCE_DIR}/eas-daemon/src
	${GLIB_INCLUDE_DIRS}
	${DBUS_GLIB_INCLUDE_DIRS}
	${EDATASERVER_INCLUDE_DIRS}
	${CHECK_INCLUDE_DIRS}
)

target_link_libraries(check_eas
	eas
	easclient
	easaccount
	eastest
	${GLIB_LDFLAGS}
	${DBUS_GLIB_LDFLAGS}
	${EDATASERVER_LDFLAGS}
	${CHECK_LDFLAGS}
)

add_check_test(check_eas)
