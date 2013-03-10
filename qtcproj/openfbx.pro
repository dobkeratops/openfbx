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

LIBS += -lGLU -lXext -lGL -lglut
TARGET = loadfbx	

SOURCES += ../*.cpp
HEADERS += ../*.h
