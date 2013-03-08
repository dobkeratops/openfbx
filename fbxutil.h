
#ifndef fbxutil_h
#define fbxutil_h

#include "fbxmath.h"
#include <string>
#include <iostream>
#include <fstream>
#include <memory>


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

class FbxUtil : public FbxMath
{
public:
    template<typename T>
    static inline bool	IsWithin(T c, T lo, T hi) {return c>=lo && c<=hi;}
    static inline bool	IsSymbolStart(char c){return IsWithin(c,'a','z') || IsWithin(c,'A','Z')|| c=='_';}
    static inline bool	IsSymbolCont(char c){return	 IsSymbolStart(c)||IsWithin(c,'0','9');}
    static inline bool	IsNumber(std::ifstream& src)
    {
        char c0=0,c1=0; src>>c0; c1=src.peek ();  src.unget();
        return	(c0=='-' && (c1=='.' || IsWithin(c1,'0','9')))
            || IsWithin(c0,'0','9');
    }
    static inline bool IsNumberStart(const char* s){
        char c0=s[0],c1=0; if (c0) c1=s[1];
        return	(c0=='-' && (c0=='.' || IsWithin(c1,'0','9')))
            || IsWithin(c0,'0','9');
    }
    // todo:IsHex() IsFloat() IsInt()
    //extern bool BeginsSymbol(char c);
    static bool	IsAlphaNumeric(char c);
    static bool	IsWhitespace(char c);
    static bool	IsSeparator(char c);
    static void	SkipLine(std::ifstream& src);
    static bool	fbxSkipComma(std::ifstream& src);
    static bool	EnterBlock(std::ifstream& src);
    static void	ExitBlock2(std::ifstream& src);
    static void	ExitBlock(std::ifstream& src);
    static void	SkipBlock(std::ifstream& src);
    static void	SkipWhitespace(std::ifstream& src);
    static void SkipWhitespaceAndSemicolonComments(std::ifstream& src);
    static void	file_trace_line(FbxStream& src);


    template<typename T>
    static T Read(std::ifstream& src) {
        T	r; src>> r; fbxSkipComma(src);
        return r;
    }

    template<typename T>
    static void	LoadNumericArray(vector<T>&	dst, FbxStream& src)
    {   while(IsNumber(src)) {
            dst.push_back(Read<T>(src));
        }
        dst.shrink_to_fit();
    }

    template<typename T>
    static bool	ReadString(T& s, std::ifstream& f)
    {
        char c; f>>c;
        if (c!='\"') { f.unget(); return false;}
        f.getline((char*)&s, sizeof(T)-1, '\"');
        fbxSkipComma(f);
        return	true;
    }
};

template<>
inline FBXM::Vector2  FbxUtil::Read(std::ifstream& src){
    return Vector2(Read<float>(src),Read<float>(src));
}
template<>
inline FbxMath::Vector3  FbxUtil::Read(std::ifstream& src){
    return Vector3(Read<float>(src),Read<float>(src),Read<float>(src));
}
template<>
inline FbxMath::Vector4  FbxUtil::Read(std::ifstream& src){
    return Vector4(Read<float>(src),Read<float>(src),Read<float>(src),Read<float>(src));
}


class IWriter : public FbxUtil {
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
    void	value(const String<N>& str) { Seperate();fprintf(fp,"\"%s\" ", str.c_str());seperated=false;}


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

    void	value(const Matrix& mat) {
        beginArray(4);
        for (int i=0; i<4; i++) {
            value(mat[i]);
        }
        endArray();
    }
    // todo, why didn't this just work from underlying std::array..
    void	value(const Vector4& v) {
        beginArray(4);
        for (int i=0; i<4; i++)
            value(v[i]);
        endArray();
    }
    void	value(const Vector3& v) {
        beginArray(3);
        for (int i=0; i<3; i++)
            value(v[i]);
        endArray();
    }
};

#endif
