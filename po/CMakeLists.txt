find_program(MSGFMT msgfmt)
find_program(MSGINIT msginit)
find_program(MSGMERGE msgmerge)
find_program(XGETTEXT xgettext)

if(MSGFMT)
	file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/LINGUAS LINGUAS REGEX "^[^#]")
	string(REGEX REPLACE "[ \t\n]+" ";" LANGUAGES "${LINGUAS}")
	foreach(lang ${LANGUAGES})
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo
			COMMAND ${MSGFMT} -o ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo
				${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po
			VERBATIM
		)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo
			DESTINATION ${LOCALE_INSTALL_DIR}/${lang}/LC_MESSAGES
			RENAME ${GETTEXT_PACKAGE}.mo
		)
		list(APPEND GMO_FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo)
	endforeach()

	add_custom_target(translations ALL DEPENDS ${GMO_FILES})
endif(MSGFMT)
