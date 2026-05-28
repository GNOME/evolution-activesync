configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/org.meego.activesyncd.service.in
	${CMAKE_CURRENT_BINARY_DIR}/org.meego.activesyncd.service
	@ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.meego.activesyncd.service
	DESTINATION ${WITH_DBUS_SERVICE_DIR}
)
