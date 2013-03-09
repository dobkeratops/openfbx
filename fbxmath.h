#ifndef FBXMATH_H
#define FBXMATH_H

#include <math.h>
#include <limits>
#include <float.h>
#include <array>
#include <vector>
#include <algorithm>
#include <string.h>

class FbxMath
{
public:
    template<typename T,int N>
    struct Vec {
        T data[N];

        //constructors
        Vec(){}
        Vec(const T& v){for (int i=0;i<N;i++) data[i]=v;}
        Vec(const T& x,const T& y){static_assert(N==2,"xy");data[0]=x;data[1]=y;}
        Vec(const T& x,const T& y,const T&z){static_assert(N==3,"xyz");data[0]=x;data[1]=y;data[2]=z;}
        Vec(const T& x,const T& y,const T&z,const T&w){static_assert(N==4,"xyzw");data[0]=x;data[1]=y;data[2]=z;data[3]=w;}
        Vec(const Vec<T,N-1>& first,const T& last){for (int i=0;i<(N-1);i++) data[i]=first[i]; data[N-1]=last;}
        Vec(const T* src) {load(src);}

        // conversions
        template<typename Y>    void load(const Y* src) {for (int i=0;i<N;i++){data[i]=src[i];}}
        template<typename Y>    void store(Y* dst) const{for (int i=0; i<N;i++){dst[i]=data[i];}}
        template<typename Y>    void load_xyz(const Y& src){static_assert(N>=3,"xyz");data[0]=src.x;data[1]=src.y;data[2]=src.z;}
        template<typename Y>    void store_xyz(Y& dst)const{static_assert(N>=3,"xyz");dst.x=data[0];dst.y=data[1];dst.z=data[2];}
        Vec<T,3> xyz()const {static_assert(N>=3,"need 3"); return Vec<T,3>(data[0],data[1],data[2]);}
        Vec<T,2> xy()const  {static_assert(N>=2,"need 2"); return Vec<T,2>(data[0],data[1]);}
        Vec<T,2> xz()const  {static_assert(N>=2,"need 2"); return Vec<T,2>(data[0],data[2]);}
        Vec<T,2> yz()const  {static_assert(N>=2,"need 2"); return Vec<T,2>(data[1],data[2]);}

        // accessors
        template<typename Y>Vec(const Y& src){static_assert(src.size()==N,""); }
        T&          operator[](int i)   {return data[i];}
        const T*    begin() const       {return data;}
        const T*    end() const         {return data+N;}
        const T&    operator[](int i)const {return data[i];}
        T* begin()          {return data;}
        T* end()            {return data+N;}
        size_t size()const  {return N;}

        // arithmetic
        inline Vec<T,N> operator*(T f)const                 { Vec<T,N> r;for (int i=0;i<N;i++) r[i]=data[i]*f;return r; }
        inline Vec<T,N> operator+(const Vec<T,N>& b)const   { Vec<T,N> r;for (int i=0;i<N;i++) r[i]=data[i]+b[i];return r;  }
        inline Vec<T,N> operator-(const Vec<T,N>& b)const   { Vec<T,N> r;for (int i=0;i<N;i++) r[i]=data[i]-b[i];return r;  }
        inline T operator|(const Vec<T,N>& b)const      {T sum=0.f; for(int i=0;i<N;i++) sum+=data[i]*b[i]; return sum;}
        inline Vec<T,N>& operator*=(const T& f)         { for (int i=0;i<N;i++) data[i]*=f; return *this; }
        inline Vec<T,N>& operator+=(const Vec<T,N>& v)  { for (int i=0;i<N;i++) data[i]+=v[i]; return *this; }
        inline Vec<T,N>& operator-=(const Vec<T,N>& v)  { for (int i=0;i<N;i++) data[i]-=v[i]; return *this; }
        inline Vec<T,N>& operator/=(const Vec<T,N>& v)  { for (int i=0;i<N;i++) data[i]/=v[i]; return *this; }
        inline T length()const              { return sqrt(*this|*this);}
        inline Vec<T,N>	normalized() const  {return (*this)*(1.f/this->length());}
        inline bool operator==(const Vec<T,N>& b) const { for (int i=0; i<N;i++) {if (data[i]!=b[i]) return false;}return true;}
        inline bool operator!=(const Vec<T,N>& b) const { return !operator==(b);}
        Vec<T,N>	operator^(const Vec<T,N>& b)const   {static_assert(N==3,"crossprod needs 3elem vec"); auto&a=*this;return	Vector3(a[1]*b[2]-a[2]*b[1],	a[2]*b[0]-a[0]*b[2],	a[0]*b[1]-a[1]*b[0]);}
        template<int A>
        static Vec<T,N> Axis(T f=1.f){Vec<T,N> r;for(int i=0;i<N;i++){r[i]=(i==A)?f:0.f;};return r;}
    };
    typedef Vec<float,2>    Vector2;
    typedef Vec<float,3>    Vector3;
    typedef Vec<float,4>    Vector4;
    struct Matrix :public Vec<Vector4,4>{
        // constructors
        Matrix(){}
        Matrix(const Vector4&a,const Vector4&b,const Vector4&c,const Vector4&d):Vec<Vector4,4>(a,b,c,d){};
        Matrix(const Vector3& a,const Vector3& b,const Vector3& c,const Vector3& d)
            :Vec<Vector4,4>(Vector4(a,0.f),Vector4(b,0.f),Vector4(c,0.f),Vector4(d,1.f)){}

        // create matrices
        static Matrix    Rot_pZ_mY_pX(const Vector3& ang);
        static Matrix    Translate(const Vector3 trans);
        static Matrix    Scale(const Vector3   scale);
        static Matrix    Srt(Vector3 scale, Vector3 rotate, Vector3 translate);
        static Matrix    LookAt(Vector3 pos,Vector3 fwd,Vector3 up);
        template<int axis> static Matrix Rotate(float angle);
        static Matrix   Identity();

        // arithmetic
        Matrix  operator*(const Matrix& b)const  {auto&m=*this; return Matrix(m * b[0], m* b[1], m * b[2], m * b[3]);}
        Vector4 operator*(const Vector4& b) const  {auto&a=*this; return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; };
        Vector3 operator*(const Vector3& b) const  {auto&a=*this; return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]).xyz(); };
        void    SetSRT(const float* ch);
    };
    //typedef std::array<Vector4,4> Matrix;
    static Vector3 Min(const Vector3& a, const Vector3& b){    return Vector3( std::min(a[0],b[0]),std::min(a[1],b[1]),std::min(a[2],b[2]) );}
    static Vector3 Max(const Vector3& a, const Vector3& b){    return Vector3( std::max(a[0],b[0]),std::max(a[1],b[1]),std::max(a[2],b[2]) );}

    //typedef std::array<Vector3,2> Extents;
    template<typename V>
    struct Extents {
        V min,max;
        Extents(const V& _min=V(FLT_MAX),const V& _max=V(-FLT_MAX)){
            min=_min;max=_max;
        };
        V centre() const{return (min+max)*0.5f;}
        V size() const{return (max-min);}
        void include(const V& v){min=Min(min,v);max=Max(max,v);};
        void include(const Extents& other) { min=Min(min,other.min);max=Max(max,other.max);}
    };
    typedef Extents<Vector2> Extents2;
    typedef Extents<Vector3> Extents3;
    typedef Extents<Vector4> Extents4;

    enum Channel_t {
        Transform_T_X,
        Transform_T_Y,
        Transform_T_Z,
        Transform_R_X,
        Transform_R_Y,
        Transform_R_Z,
        Transform_S_X,
        Transform_S_Y,
        Transform_S_Z,
        NumChannels
    };


    // todo - util class derived from this
    template<int N>
    class	String : public std::array<char,N> {
    public:
        String(const char* src) {
            auto x=10;
            int len=strlen(src);
            if (len>(N-1)) len=N-1;
            memcpy(&this->at(0), src,len);
            this->at(len)=0;
        }
        String() { this->at(0)=0;}
        operator char* () { return &this->at(0);}
        const char* c_str() const { return &this->at(0);}
        bool operator==(const char* txt) { return !strcmp(&this->at(0),txt);}
    };

    #ifdef TEST
        #define fbx_printf printf
        #else
            static inline void fbx_printf(const char*,...) { };
        #endif


    static void    fbx_printf(const Vector3& v) {
        fbx_printf("[%.5f %.5f %.5f]",v[0],v[1],v[2]);
    }
    static void    fbx_printf(const Vector4& v) {
        fbx_printf("[%.5f %.5f %.5f %.5f]",v[0],v[1],v[2],v[3]);
    }

    static void    fbx_printf(const Matrix& m) {
        int	i;
        fbx_printf("[");
        for (i=0; i<4; i++) {
         fbx_printf(m[i]);
        }
        fbx_printf("]\n");
    }

    template<int N>
    void	fbx_printf(const String<N>& str) {
        fbx_printf(&str[0]);
    }
};

inline FbxMath::Matrix FbxMath::Matrix::Identity() {
    return Matrix(Vector4::Axis<0>(),Vector4::Axis<1>(),Vector4::Axis<2>(),Vector4::Axis<3>());
}

template<int axis>
FbxMath::Matrix	FbxMath::Matrix::Rotate(float angle) {
    auto u=(axis==0)?1:0;
    auto v=(axis==2)?1:2;
    float s=sin(angle),c=cos(angle);
    auto ret=Identity();
    ret[u][u]=c;
    ret[u][v]=s;
    ret[v][u]=-s;
    ret[v][v]=c;
    return ret;
};


#endif // FBXMATH_H
