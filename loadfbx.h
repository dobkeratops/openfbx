#ifndef loadfbx_h
#define loadfbx_h

#include "fbxscene.h"

void
LoadFbx(FbxScene*   scn, std::ifstream& src);


#ifdef TEST
void fbx_dumpline(ifstream & src) {
	return;
	/* CAUTION THIS APPEARS TO BREAK ON WINDOWS */
	int pos=src.tellg();
	char tmp[1024];
	src.getline(tmp,1023);
	fbx_printf("SRC:%s\n", tmp);
	src.seekg(pos);
}
#else
inline void fbx_dumpline(std::ifstream & ) {
}
#endif



#endif

