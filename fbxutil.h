
#ifndef fbxutil_h
#define fbxutil_h

#include <math.h>
#include <array>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#include "fbxmath.h"

#ifndef ASSERT
    #define ASSERT(x) { if (!(x)) { FBXM::fbx_printf("failed %s:%d %s",__FILE__,__LINE__,#x); }}
#endif

typedef ifstream FbxStream;

template<typename T, typename... Args>
std::unique_ptr<T> fbxMakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
//typedef FBXM::Matrix FbxMatrix;
template<typename T> inline T*	fbxAppend(std::vector<T>& list, int numToAdd=1) { int oldSize=list.size(); list.resize(oldSize+numToAdd); return &list[oldSize];}

template<typename T>
inline bool	le(T a,T b, T c) {return a<=b && b<=c;};
inline bool	fbxIsSymbolStart(char c)	{return le('a',c,'z') || le('A',c,'Z')|| c=='_';}
inline bool	fbxIsSymbolCont(char c)	{return	 fbxIsSymbolStart(c)||le('0',c,'9');}




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
    T	r; src>> r; fbxSkipComma(src);
    return r;
}
template<>
inline FBXM::Vector2  Read(std::ifstream& src){
    return fbxvec2(Read<float>(src),Read<float>(src));
}
template<>
inline FBXM::Vector3  Read(std::ifstream& src){
    return fbxvec3(Read<float>(src),Read<float>(src),Read<float>(src));
}
template<>
inline FBXM::Vector4  Read(std::ifstream& src){
    return fbxvec4(Read<float>(src),Read<float>(src),Read<float>(src),Read<float>(src));
}

template<typename T>
void	FbxLoadNumericArray(vector<T>&	dst, FbxStream& src)
{   while(fbxIsNumber(src)) {
        dst.push_back(Read<T>(src));
    }
    dst.shrink_to_fit();
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

class IWriter {
public:
    //TODO, move to abstract interface.
    FILE* fp;
    IWriter(FILE* _fp) {fp=_fp;seperated=false;}
    ~IWriter() {}
    //todo - indent..
    int x;
    bool seperated;
    void	beginMap() { Seperate(),fprintf(fp, "{\n");seperated=true;}
    void	beginKeyValue(const char* key) {Seperate(); fprintf(fp, "%s:", key); seperated=true;}
    void	endKeyValue() {}
    template<typename T>
    void	keyValue(const char* key, const T& v) { this->beginKeyValue(key); value(v);fprintf(fp,"\n"); endKeyValue();seperated=false;}
    void	endMap() { fprintf(fp, "}\n");seperated=false;}

    void	beginArray(int size) { Seperate();fprintf(fp, "[");seperated=true;}
    void	endArray() { fprintf(fp, "]");seperated=false;}

    void    Seperate() { if (!seperated){fprintf(fp,",");seperated=true;};}
    void	value(int x) { Seperate();fprintf(fp,"%d", x);seperated=false;}
    void	value(float f) { Seperate();fprintf(fp,"%.5f", f);seperated=false;}
    void	value(const char* str) {
        Seperate();
        fprintf(fp,"\"%s\" ", str);
        seperated=false;
    }

    template<typename T>
    void	keyValueArray(const char* key, const std::vector<T>& src) {
        beginKeyValue(key);
        beginArray(src.size());
        for (auto&n : src) { value(this,n); }
        endArray();
        endKeyValue();
    }

    template<typename T>
    void	keyValuePtrArray(const char* key, const std::vector<T>& src) {
        beginKeyValue(key);
        beginArray(src.size());
        for (auto pn : src) { value(this,*pn); }
        endArray();
        endKeyValue();
    }

    template<int N>
    void	value(const FBXM::String<N>& str) { Seperate();fprintf(fp,"\"%s\" ", str.c_str());seperated=false;}


    template<typename T,int N>
    void	value(const std::array<T,N>& a) {
        beginArray(a.size());
        int	i;
        for (i=0; i<a.size(); i++) value(a[i]);
        endArray();
    }

    template<typename T>
    void	value(const std::vector<T>& a) {
        beginArray(a.size());
        int	i;
        for (i=0; i<a.size(); i++) value(a[i]);
        endArray();
    }

    void	value(const FBXM::Matrix& mat) {
        beginArray(4);
        for (int i=0; i<4; i++) {
            value(mat[i]);
        }
        endArray();
    }
    // todo, why didn't this just work from underlying std::array..
    void	value(const FBXM::Vector4& v) {
        beginArray(4);
        for (int i=0; i<4; i++)
            value(v[i]);
        endArray();
    }
    void	value(const FBXM::Vector3& v) {
        beginArray(3);
        for (int i=0; i<3; i++)
            value(v[i]);
        endArray();
    }
};

#endif
