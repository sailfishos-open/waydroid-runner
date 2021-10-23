# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = waydroid-runner

# PREFIX
isEmpty(PREFIX) {
    PREFIX = /usr
}

target.path = $$PREFIX/bin

CONFIG += sailfishapp sailfishapp_i18n

DEFINES += QT_COMPOSITOR_QUICK
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += quick qml svg
QT += quick-private

QT += dbus

QT += compositor

SOURCES += \
    src/main.cpp \
    src/qmlcompositor.cpp \
    src/runner.cpp

OTHER_FILES += \
    qml/main.qml \
    qml/MainPage.qml \
    qml/WindowContainer.qml \

DISTFILES += \
    rpm/waydroid-runner.spec

HEADERS += \
    src/qmlcompositor.h \
    src/runner.h

icons.files = icons
icons.path = $$PREFIX/share/$${TARGET}
INSTALLS += icons

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172 256x256 512x512
DISTFILES += $${TARGET}.desktop

# translations
TRANSLATIONS += \
    translations/$${TARGET}-de_DE.ts \
