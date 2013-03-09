
#ifndef fbxutil_h
#define fbxutil_h

#include "fbxmath.h"
#include <string>
#include <iostream>
#include <fstream>
#include <memory>


#ifndef ASSERT
    #define ASSERT(x) { if (!(x)) { fbx_printf("failed %s:%d %s",__FILE__,__LINE__,#x); }}
#endif


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
    static inline bool	within(T c, T lo, T hi) {return c>=lo && c<=hi;}
    static inline bool	IsSymbolStart(char c){return within(c,'a','z') || within(c,'A','Z')|| c=='_';}
    static inline bool	IsSymbolCont(char c){return	 IsSymbolStart(c)||within(c,'0','9');}
    static bool IsNumberStart(const char* src);
    static bool	IsAlphaNumeric(char c);
    static bool	IsWhitespace(char c);
    static bool	IsSeparator(char c);
    class Stream : public std::ifstream{
        // todo- mayinclude line,nesting depth?
    public:
//        Stream(std::ifstream& f) :src(f){};
        void	SkipLine();
        bool	fbxSkipComma();
        bool	EnterBlock();
        void	ExitBlock();
        void	SkipBlock();
        void	SkipWhitespace();
        void SkipWhitespaceAndSemicolonComments();
        void	file_trace_line();
        bool IsNumber();
        template<typename T>
        T Read() {
            T	r; *this>> r; fbxSkipComma();
            return r;
        }
        Stream& operator>>(float& f) {
            std::ifstream::operator>>(f);
            fbxSkipComma();
            return*this;
        }
        Stream& operator>>(int& n) {
            std::ifstream::operator>>(n);
            fbxSkipComma();
            return*this;
        }

        template<typename T>
        void	LoadNumericArray(std::vector<T>&	dst)
        {   while(IsNumber()) {
                auto v=Read<T>();
                std::cout<<v;
                dst.push_back(v);
            }
            dst.shrink_to_fit();
        }
        template<typename T>
        bool	ReadString(T& s)
        {
            //SkipWhitespace();
            char c; *this>>c;
            if (c!='\"') { unget(); return false;}
            getline((char*)&s, sizeof(T)-1, '\"');
            fbxSkipComma();
            return	true;
        }
        template<typename T,int N>
        inline Stream& operator>>(Vec<T,N>& dst){
            for (int i=0; i<N;i++) (*this)>>dst[i];
            return *this;
        }
/*
        inline Stream& operator>>(Vector2& dst){
            return (*this)>>dst[0]>>dst[1]>>dst[2];
        }
        inline Stream& operator>>(Vector3& dst){
            return (*this)>>dst[0]>>dst[1]>>dst[2];

        }
        inline Stream& operator>>(Vector4& dst){
            return (*this)>>dst[0]>>dst[1]>>dst[2]>>dst[3];
        }
*/

    };
};



template<int N>
inline FbxUtil::Stream& operator>>(FbxUtil::Stream& src,FbxUtil::String<N>& dst) {
    src.ReadString(dst);
    return src;
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
