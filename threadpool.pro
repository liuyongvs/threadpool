TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    threadpool.c

HEADERS += \
    threadpool.h

QMAKE_CXXFLAGS += -std=c++0x -pthread
LIBS += -pthread
