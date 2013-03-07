UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
	LDFLAGS=-framework Carbon -framework OpenGL -framework GLUT
	CXX = clang++ -std=c++11 -stdlib=libc++
else
	LDFLAGS= -lGL -lglut
	CXX = clang++ -std=c++11
endif
all:
	$(CXX) *.cpp -I../ut -I../gfx -DFBXVIEWER_MAIN -o bin/loadfbx $(LDFLAGS)
	bin/loadfbx data/test.fbx
