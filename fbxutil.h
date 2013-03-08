
#ifndef fbxutil_h
#define fbxutil_h

#include <math.h>
#include <array>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#ifndef ASSERT
    #define ASSERT(x) { if (!(x)) { fbx_printf("failed %s:%d %s",__FILE__,__LINE__,#x); }}
#endif

#ifdef TEST
#define fbx_printf printf
#else
inline void fbx_printf(const char*,...) { };
#endif

//typedef FBXM::Matrix FbxMatrix;
template<typename T> inline T*	fbxAppend(std::vector<T>& list, int numToAdd=1) { int oldSize=list.size(); list.resize(oldSize+numToAdd); return &list[oldSize];}

template<typename T>
inline bool	le(T a,T b, T c) {return a<=b && b<=c;};
inline bool	fbxIsSymbolStart(char c)	{return le('a',c,'z') || le('A',c,'Z')|| c=='_';}
inline bool	fbxIsSymbolCont(char c)	{return	 fbxIsSymbolStart(c)||le('0',c,'9');}
template<int N>
class	FbxString : public std::array<char,N> {
public:

    FbxString(const char* src) {
        auto x=10;
        int len=strlen(src);
        if (len>(N-1)) len=N-1;
        memcpy(&this->at(0), src,len);
        this->at(len)=0;
    }
//	FbxString<N>& operator=(const char* src) { if (strlen(src)<(N-1)) strcpy(&(*this)[0], src); return *this;}
    FbxString() { this->at(0)=0;}
    operator char* () { return &this->at(0);}
    const char* c_str() const { return &this->at(0);}
    bool operator==(const char* txt) { return !strcmp(&this->at(0),txt);}
};




template<typename T>
inline bool	IsWithin(T c, T lo, T hi) {return c>=lo && c<=hi;}
inline bool	IsSymbolStart(char c){return IsWithin(c,'a','z') || IsWithin(c,'A','Z')|| c=='_';}
inline bool	IsSymbolCont(char c){return	 IsSymbolStart(c)||IsWithin(c,'0','9');}
inline bool	IsNumber(std::ifstream& src)
{
    char c0=0,c1=0; src>>c0; c1=src.peek ();  src.unget();
    return	(c0=='-' && (c1=='.' || IsWithin(c1,'0','9')))
        || IsWithin(c0,'0','9');
}
inline bool IsNumberStart(const char* s){
    char c0=s[0],c1=0; if (c0) c1=s[1];
    return	(c0=='-' && (c0=='.' || IsWithin(c1,'0','9')))
        || IsWithin(c0,'0','9');
}
// todo:IsHex() IsFloat() IsInt()
//extern bool BeginsSymbol(char c);
extern bool	IsAlphaNumeric(char c);
extern bool	IsWhitespace(char c);
extern bool	IsSeparator(char c);
extern int	ReadInt(FILE* fp);
extern float	ReadFloat(FILE* fp);
extern void	ReadLine(FILE* fp, char* line, int maxlen);
extern void	SkipLine(std::ifstream& src);
extern bool	fbxSkipComma(std::ifstream& src);
extern bool ReadString(FILE* fp, char* txt, int maxlan);
extern bool	ReadToken(FILE*, char* tok, int maxLen);
extern void	ExitBlock(FILE* fp);
extern bool	EnterBlock(
                   FILE* fp,
                   char* blockname,
                   int maxBlockName,
                   char* insname,
                   int maxInsName);
template<typename T>
T Load(std::ifstream& src);


extern bool	ExitBlock(FILE*	fp, char* blockName,int maxBlockName, char* insname, int maxInsName);
bool	EnterBlock(std::ifstream& src);
void	ExitBlock2(std::ifstream& src);
void	ExitBlock(std::ifstream& src);
void	SkipBlock(std::ifstream& src);
void	SkipWhitespace(std::ifstream& src);

inline bool	fbxIsNumber(std::ifstream& src)
{
    SkipWhitespace(src);
    while (src.peek()==',') src.get();
    char c0=0,c1=0; src>>c0; c1=src.peek ();  src.unget();
    return	(c0=='-' && (c1=='.' || le('0',c1,'9')))
        || le('0',c0,'9');
}

template<typename T>
T Read(std::ifstream& src) {
    T	r; src>> r; fbxSkipComma(src); return r;
}

extern bool	EnterBlock(FILE* fp);
extern float ReadFloat2(FILE* fp);
template<typename T>
bool	ReadString(T& s, std::ifstream& f)
{
    char c; f>>c;
    if (c!='\"') { f.unget(); return false;}
    f.getline((char*)&s, sizeof(T)-1, '\"');
    fbxSkipComma(f);
    return	true;
}


#endif
