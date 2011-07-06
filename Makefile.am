ACLOCAL_AMFLAGS = -I m4 -I m4-repo
#SUBDIRS = libeasmail/tests

if ENABLE_SYNCEVOLUTION
  SYNCEVOLUTION_DIR = syncevolution
endif

if ENABLE_MEEGO_UI
  MEEGO_DIR = meego
endif

SUBDIRS = . logger libeasaccount/src libeasaccount/tests eas-daemon/libeas libeastest/src libeassync/src libeasmail/src eas-daemon/src eas-daemon/tests eas-daemon/data check_tests camel $(SYNCEVOLUTION_DIR) $(MEEGO_DIR)

EXTRA_DIST = autogen.sh
