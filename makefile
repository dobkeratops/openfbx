ifeq ($(shell uname),Darwin)
	LDFLAGS=-framework Carbon -framework OpenGL -framework GLUT
	CXX = clang++ -std=c++11 -stdlib=libc++
else
	LDFLAGS= -lGL -lglut
	CXX = clang++ -std=c++11
endif

TARGET=loadfbx
SRC=$(wildcard ./*.cpp)
SRC_H=$(wildcard ./*.h)

OBJ=$(SRC:.cpp=.o)

$(TARGET): $(OBJ) $(SRC_H)
	$(CXX) -o $@ $(SRC) -I../ut -I../gfx $(LDFLAGS)
	./$@ data/test.fbx
clean:
	rm -f *.o
	rm $(TARGET)
