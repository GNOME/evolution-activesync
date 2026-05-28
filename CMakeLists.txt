# evolution-activesync build script

cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

project(evolution-activesync
	VERSION 0.92
	LANGUAGES C)
set(PROJECT_BUGREPORT "https://gitlab.gnome.org/GNOME/evolution-activesync/issues/")

set(GETTEXT_PACKAGE ${PROJECT_NAME})
set(GETTEXT_PO_DIR ${CMAKE_SOURCE_DIR}/po)

# ******************************
# Library versioning
# ******************************
set(LIBEASACCOUNT_CURRENT 0)
set(LIBEASACCOUNT_REVISION 0)
set(LIBEASACCOUNT_AGE 0)

set(LIBEASCLIENT_CURRENT 0)
set(LIBEASCLIENT_REVISION 0)
set(LIBEASCLIENT_AGE 0)

# Keep these two definitions in agreement.
set(glib_minimum_version 2.68)
set(glib_encoded_version GLIB_VERSION_2_68)

# Keep these two definitions in agreement.
set(soup_minimum_version 3.0)
set(soup_encoded_version SOUP_VERSION_3_0)

add_definitions(-DGLIB_VERSION_MAX_ALLOWED=${glib_encoded_version})
add_definitions(-DSOUP_VERSION_MAX_ALLOWED=${soup_encoded_version})
add_definitions(-DGLIB_VERSION_MIN_REQUIRED=${glib_encoded_version})
add_definitions(-DSOUP_VERSION_MIN_REQUIRED=${soup_encoded_version})

# Load modules from the source tree
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_SKIP_RPATH OFF CACHE BOOL INTERNAL)
set(CMAKE_SKIP_BUILD_RPATH OFF CACHE BOOL INTERNAL)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF CACHE BOOL INTERNAL)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON CACHE BOOL INTERNAL)

# CMake modules
include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)

# Project custom modules
include(PrintableOptions)

add_printable_variable(LIB_SUFFIX "Library directory suffix, usually defined to '64' for x86_64 systems" "")
add_printable_variable_bare(CMAKE_INSTALL_PREFIX)
add_printable_variable_path(BIN_INSTALL_DIR "Install directory for binary files, defaults to CMAKE_INSTALL_PREFIX/bin" "")
add_printable_variable_path(INCLUDE_INSTALL_DIR "Install directory for header files, defaults to CMAKE_INSTALL_PREFIX/include" "")
add_printable_variable_path(LIB_INSTALL_DIR "Install directory for library files, defaults to CMAKE_INSTALL_PREFIX/lib{LIB_SUFFIX}" "")
add_printable_variable_path(LIBEXEC_INSTALL_DIR "Install directory for library executable files, defaults to CMAKE_INSTALL_PREFIX/libexec" "")
add_printable_variable_path(SHARE_INSTALL_PREFIX "Install directory for shared files, defaults to CMAKE_INSTALL_PREFIX/share" "")
add_printable_variable_path(LOCALE_INSTALL_DIR "Install directory for locale files, defaults to SHARE_INSTALL_PREFIX/locale" "")
add_printable_variable_path(SYSCONF_INSTALL_DIR "Install directory for system configuration files, defaults to CMAKE_INSTALL_PREFIX/etc" "")

macro(ensure_default_value _var _defvalue)
	if(${_var} STREQUAL "")
		set(${_var} ${_defvalue})
	endif()
endmacro()

ensure_default_value(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
ensure_default_value(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
ensure_default_value(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
ensure_default_value(LIBEXEC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/libexec")
ensure_default_value(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share")
ensure_default_value(LOCALE_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/locale")
ensure_default_value(SYSCONF_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc")

# ******************************
# Special directories
# ******************************
set(privincludedir "${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}")
set(privlibdir "${LIB_INSTALL_DIR}/${PROJECT_NAME}")
set(privlibexecdir "${LIBEXEC_INSTALL_DIR}/${PROJECT_NAME}")

set(CMAKE_INSTALL_RPATH "${privlibdir}")
if(ENABLE_CAMEL_BACKEND)
	list(APPEND CMAKE_INSTALL_RPATH "${EVOLUTION_privlibdir}")
endif(ENABLE_CAMEL_BACKEND)

add_printable_variable_path(WITH_DBUS_SERVICE_DIR "Set directory for D-Bus service files" "${SHARE_INSTALL_PREFIX}/dbus-1/services")

# ******************************
# Dependencies
# ******************************

include(CheckTarget)
include(GLibTools)
include(PkgConfigEx)
include(SetupBuildFlags)
include(UninstallTarget)

add_printable_option(ENABLE_TESTS "Build unit tests" ON)
add_printable_option(ENABLE_CAMEL_BACKEND "Build the Camel e-mail backend" ON)
add_printable_option(ENABLE_MAINTAINER_MODE "Enable maintainer mode" OFF)

include(FindGettext)
include(I18n)

pkg_check_modules(GLIB REQUIRED glib-2.0>=${glib_minimum_version})
pkg_check_modules(GIO REQUIRED gio-2.0>=${glib_minimum_version})
pkg_check_modules(GOBJECT REQUIRED gobject-2.0>=${glib_minimum_version})
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)
pkg_check_modules(EDATASERVER REQUIRED libedataserver-1.2)
pkg_check_modules(LIBXML REQUIRED libxml-2.0)
pkg_check_modules(SOUP REQUIRED libsoup-3.0>=${soup_minimum_version})
pkg_check_modules(WBXML REQUIRED "libwbxml2>=0.11")
pkg_check_modules(LIBICAL REQUIRED libical)
pkg_check_modules(LIBEBOOK REQUIRED libebook-1.2)
pkg_check_modules(LIBEBACKEND REQUIRED libebackend-1.2)

if(ENABLE_TESTS)
	pkg_check_modules(CHECK REQUIRED check)
endif(ENABLE_TESTS)

if(ENABLE_CAMEL_BACKEND)
	pkg_check_modules(CAMEL REQUIRED "camel-1.2>=3.23.2")
	pkg_check_modules(LIBEMAIL_ENGINE REQUIRED libemail-engine)
	pkg_check_modules(EVOLUTION_MAIL REQUIRED evolution-mail-3.0)
	pkg_check_modules(EVOLUTION_SHELL REQUIRED evolution-shell-3.0)

	pkg_check_variable(camel_providerdir camel-1.2 camel_providerdir)
	pkg_check_variable(eds_moduledir libebackend-1.2 moduledir)
	pkg_check_variable(evo_moduledir evolution-shell-3.0 moduledir)
	pkg_check_variable(EVOLUTION_privlibdir evolution-data-server-1.2 privlibdir)
	list(APPEND CMAKE_INSTALL_RPATH "${EVOLUTION_privlibdir}")
endif(ENABLE_CAMEL_BACKEND)

# Find ssh-askpass
find_program(ASKPASS
	NAMES ssh-askpass gnome-ssh-askpass
	PATHS /usr/libexec /usr/libexec/ssh /usr/libexec/openssh
	      ${LIBEXEC_INSTALL_DIR} ${LIBEXEC_INSTALL_DIR}/ssh ${LIBEXEC_INSTALL_DIR}/openssh
)
if(NOT ASKPASS)
	set(ASKPASS "ssh-askpass")
endif()

# Check for new-style libxml2 buffer API
check_function_exists(xmlOutputBufferGetContent HAVE_LIBXML2_NEW_BUFFER)

setup_build_flags(${ENABLE_MAINTAINER_MODE})

# ******************************
# config.h
# ******************************
configure_file(
	${CMAKE_SOURCE_DIR}/config.h.in
	${CMAKE_BINARY_DIR}/config.h
	@ONLY
)
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_BINARY_DIR})

# ******************************
# GSettings schemas
# ******************************
add_custom_target(evolution-activesync-schemas ALL)
add_gsettings_schemas(evolution-activesync-schemas
	${CMAKE_SOURCE_DIR}/org.meego.activesyncd.gschema.xml
	${CMAKE_SOURCE_DIR}/org.meego.activesyncd.account.gschema.xml
)

# ******************************
# Subdirectories
# ******************************
add_subdirectory(libeasaccount/src)
add_subdirectory(libeasclient)
add_subdirectory(eas-daemon)

if(ENABLE_CAMEL_BACKEND)
	add_subdirectory(libevoeas)
	add_subdirectory(camel)
	add_subdirectory(collection)
	add_subdirectory(configuration)
endif(ENABLE_CAMEL_BACKEND)

if(ENABLE_TESTS)
	add_subdirectory(libeastest/src)
	add_subdirectory(check_tests)
endif(ENABLE_TESTS)

add_subdirectory(po)

print_build_options()
