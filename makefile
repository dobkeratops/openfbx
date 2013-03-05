all:
	clang++ *.cpp -I../ut -I../gfx -std=c++11 -DFBXVIEWER_MAIN -o bin/loadfbx -lGL -lglut
	bin/loadfbx data/test.fbx
