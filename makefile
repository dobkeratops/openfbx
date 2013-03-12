ifeq ($(shell uname),Darwin)
	LDFLAGS=-framework Carbon -framework OpenGL -framework GLUT
	CXX = clang++ -std=c++11 -stdlib=libc++
else
	LDFLAGS= -lGL -lglut
	CXX = clang++ -std=c++11
endif
TARGET=loadfbx

$(TARGET):
	$(CXX) *.cpp -I../ut -I../gfx -o $(TARGET) $(LDFLAGS)
	./$(TARGET) data/test.fbx
clean:
	rm -f *.o
	rm $(TARGET)
