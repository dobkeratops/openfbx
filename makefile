ifeq ($(shell uname),Darwin)
	LDFLAGS=-framework Carbon -framework OpenGL -framework GLUT
	CXX = clang++ -std=c++11 -stdlib=libc++
else
	LDFLAGS= -lGL -lglut
	CXX = clang++ -std=c++11
endif
TARGET=loadfbx

SRC=$(wildcard ./*.cpp)
OBJ=$(SRC:.cpp=.o)
$(TARGET): $(OBJ)
	$(CXX) $^ -I../ut -I../gfx -o $@ $(LDFLAGS)
	./$@ data/test.fbx
clean:
	rm -f *.o
	rm $(TARGET)
