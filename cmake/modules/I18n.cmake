# I18n.cmake
#
# Macros to easily use Gettext translations capabilities
#

include(FindGettext)
include(CMakeParseArguments)

macro(i18n_merge_file _source _target _po_dir)
	cmake_parse_arguments(_I18N "" "TYPE" "EXTRA_ARGS" ${ARGN})
	if (NOT DEFINED _I18N_TYPE)
		set(_I18N_TYPE "desktop")
	elseif(NOT ((${_I18N_TYPE} STREQUAL "desktop") OR (${_I18N_TYPE} STREQUAL "xml")))
		message(FATAL_ERROR "Wrong type supplied, only 'desktop' and 'xml' are allowed, given: '${_I18N_TYPE}'")
	endif()
	add_custom_command(
		OUTPUT ${_target}
		DEPENDS ${_source} "${_po_dir}/LINGUAS"
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
			--${_I18N_TYPE}
			--template=${_source}
			-d ${_po_dir}
			-o ${_target}
			${_I18N_EXTRA_ARGS}
	)
endmacro(i18n_merge_file)
