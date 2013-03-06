TEMPLATE = app
CONFIG += console
CONFIG -= qt
QMAKE_CXX = clang++
QMAKE_CXXFLAGS+= -std=c++11
QMAKE_CXXFLAGS_WARN_ON +=\
	-Wno-unused-variable\
	-Wno-unused-parameter\
	-Wno-unused-function\
	-Wno-char-subscripts\

DEFINES += LINUX
DEFINES += FBXVIEWER_MAIN
LIBS += -lGLU -lXext -lGL -lglut
DESTDIR = build/debug
OBJECTS_DIR = build/debug/obj
TARGET = loadfbx	

SOURCES += ../*.cpp
HEADERS += ../*.h
