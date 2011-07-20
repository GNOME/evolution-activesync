VERSION = 0.0.1
TEMPLATE = lib 

OBJECTS_DIR = .obj
MOC_DIR = .moc

CONFIG += \
    link_pkgconfig \
    qt \
    plugin \
    debug \
    warn_on

PKGCONFIG += \
    mlite \
    libedataserver-1.2

QT += declarative xml

HEADERS = \
    Config.hpp \
    ConfigModel.hpp \
    EmailAccount.hpp \
    Plugin.hpp

SOURCES = \
    Config.cpp \
    ConfigModel.cpp \
    EmailAccount.cpp \
    Plugin.cpp

qmlfiles.files += *.qml settings/
qmlfiles.path += $$INSTALL_ROOT/usr/share/meego-ux-settings/ActiveSync #/$$TARGET
settings_desktop.files += activesync-settings.desktop
settings_desktop.path += $$INSTALL_ROOT/usr/share/meego-ux-settings/apps

OTHER_FILES += qmldir

qmldir.files += qmldir
qmldir.path = $$[QT_INSTALL_IMPORTS]/MeeGo/ActiveSync

TARGET = $$qtLibraryTarget(ActiveSync)
target.path = $$[QT_INSTALL_IMPORTS]/MeeGo/ActiveSync

INSTALLS += target qmldir qmlfiles settings_desktop

TRANSLATIONS += *.qml
PROJECT_NAME = meego-ux-settings-activesync

# Custom "dist" target that supports generation of translation
# related files.
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += git clone . $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}/.git &&
dist.commands += rm -f $${PROJECT_NAME}-$${VERSION}/.gitignore &&
dist.commands += mkdir -p $${PROJECT_NAME}-$${VERSION}/ts &&
dist.commands += lupdate $${TRANSLATIONS} -ts $${PROJECT_NAME}-$${VERSION}/ts/$${PROJECT_NAME}.ts &&
dist.commands += tar jcpvf $${PROJECT_NAME}-$${VERSION}.tar.bz2 $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}
QMAKE_EXTRA_TARGETS += dist
