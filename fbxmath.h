#ifndef FBXMATH_H
#define FBXMATH_H

#include <math.h>
#include <limits>
#include <float.h>
#include <array>
#include <vector>
#include <algorithm>

class FbxMath
{
public:
    struct Vector2 :public std::array<float,2> {
        Vector2(){};
        void set(float x,float y) {(*this)[0]=x;(*this)[1]=y;}
        Vector2(float x, float y) {this->set(x,y);}
        explicit Vector2(float f) :Vector2(f,f){}
    };
    struct Vector3 :public std::array<float,3> {
        Vector3(){};
        Vector3(float x, float y,float z) {(*this)[0]=x;(*this)[1]=y;(*this)[2]=z;}
        explicit Vector3(float f) :Vector3(f,f,f){};
    };
    struct Vector4 :public std::array<float,4> {
        Vector4(){};
        Vector4(float x, float y,float z,float w) {(*this)[0]=x;(*this)[1]=y;(*this)[2]=z;(*this)[3]=w;}
        Vector4(const Vector3& xyz,float w) :Vector4(xyz[0],xyz[1],xyz[2],w){}
        explicit Vector4(float f) :Vector4(f,f,f,f){}
    };
    struct Matrix: public std::array<Vector4,4> {
        Matrix(){}
        Matrix(const Vector4& a,const Vector4& b,const Vector4& c,const Vector4& d) {
            (*this)[0]=a; (*this)[1]=b; (*this)[2]=c;(*this)[3]=d;
        }
        Matrix(const Vector3& a,const Vector3& b,const Vector3& c,const Vector3& d)
            :Matrix(Vector4(a,0.f),Vector4(b,0.f),Vector4(c,0.f),Vector4(d,1.f)){}
    };
    //typedef std::array<Vector4,4> Matrix;
    inline static Vector3 Min(const Vector3&a,const Vector3& b);
    inline static Vector3 Max(const Vector3&a,const Vector3& b);
    typedef std::array<Vector3,2> Extents;
    struct Extents3 {
        Vector3 min,max;
        Extents3(const Vector3& _min=Vector3(FLT_MAX,FLT_MAX,FLT_MAX),const Vector3& _max=Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX)){
            min=_min;max=_max;
        };
        void include(const Vector3& v){min=Min(min,v);max=Max(max,v);};
    };

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

    static void    EvalSRT(Matrix* dst, const float* ch);
};

typedef FbxMath FBXM;
inline FBXM::Vector2 fbxvec2(float x, float y) {return FBXM::Vector2(x,y);}
inline FBXM::Vector3 fbxvec3(float x, float y, float z) {return FBXM::Vector3(x,y,z);}
inline FBXM::Vector4 fbxvec4(float x, float y, float z,float w) {return FBXM::Vector4(x,y,z,w);}
inline FBXM::Matrix fbxmatrix(const FBXM::Vector4& ax, const FBXM::Vector4& ay,const FBXM::Vector4& az,const FBXM::Vector4& aw) {return FBXM::Matrix(ax,ay,az,aw);}

//inline FBXM::Vector3 FbxVector3(float x,float y,float z) {return FBXM::Vector3({{x,y,z}});}
using namespace std;

inline FBXM::Vector3 operator*(const FBXM::Vector3& a,float f)  { return fbxvec3(a[0]*f,a[1]*f,a[2]*f); }
inline FBXM::Vector3 operator+(const FBXM::Vector3& a,const FBXM::Vector3& b)  { return fbxvec3(a[0]+b[0], a[1]+b[1], a[2]+b[2]); }
inline FBXM::Vector3 operator-(const FBXM::Vector3& a,const FBXM::Vector3& b)  { return fbxvec3(a[0]-b[0], a[1]-b[1], a[2]-b[2]); }
inline FBXM::Vector3& operator*=(FBXM::Vector3& a,float f)  { a[0]=a[0]*f; a[1]=a[1]*f; a[2]=a[2]*f; return a; }
inline FBXM::Vector4 operator*(const FBXM::Vector4& a,float f)  { return fbxvec4(a[0]*f,a[1]*f,a[2]*f,a[3]*f); }
inline FBXM::Vector4 operator+(const FBXM::Vector4& a,const FBXM::Vector4& b)  { return fbxvec4(a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3]); }
inline FBXM::Vector4 operator-(const FBXM::Vector4& a,const FBXM::Vector4& b)  { return fbxvec4(a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3]); }
inline FBXM::Vector4 operator+=(FBXM::Vector4& a,const FBXM::Vector4& b)  { a[0]+=b[0];a[1]+=b[1]; a[2]+=b[2]; a[3]+=b[3]; return a; }

inline FBXM::Vector4 operator*(const FBXM::Matrix& a,const FBXM::Vector4& b)  { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; };
inline FBXM::Matrix  operator*(const FBXM::Matrix& m,const FBXM::Matrix& b)  { return fbxmatrix(m * b[0], m* b[1], m * b[2], m * b[3]);}
inline FBXM::Vector3	operator^(const FBXM::Vector3& a,const FBXM::Vector3& b){return	fbxvec3(a[1]*b[2]-a[2]*b[1],	a[2]*b[0]-a[0]*b[2],	a[0]*b[1]-a[1]*b[0]);}
inline float			operator|(const FBXM::Vector4& a,const FBXM::Vector4& b) {return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];}
inline float			operator|(const FBXM::Vector3& a,const FBXM::Vector3& b) {return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
inline float length(const FBXM::Vector3& v){ return sqrt(v|v);}
inline FBXM::Vector3	normalize(const FBXM::Vector3& a) {return a*(1.f/length(a));}

inline FBXM::Vector4 FbxAxisX (float fval=1.f) {return FBXM::Vector4(fval,0.f,0.f,0.f);}
inline FBXM::Vector4 FbxAxisY (float fval=1.f) {return FBXM::Vector4(0.f,fval,0.f,0.f);}
inline FBXM::Vector4 FbxAxisZ (float fval=1.f) {return FBXM::Vector4(0.f,0.f,fval,0.f);}
inline FBXM::Vector4 FbxAxisW (float fval=1.f) {return FBXM::Vector4(0.f,0.f,0.f,fval);}
inline FBXM::Matrix FbxMatrixIdentity() {
    return fbxmatrix(FbxAxisX(),FbxAxisY(),FbxAxisZ(),FbxAxisW());
}

template<int u, int v>
FBXM::Matrix	FbxMatrixRotate(float angle) {
    float s=sin(angle),c=cos(angle);
    auto ret=FbxMatrixIdentity();
    ret[u][u]=c;
    ret[u][v]=s;
    ret[v][u]=-s;
    ret[v][v]=c;
    return ret;
};
void	EvalSRT(FBXM::Matrix* dst, const float* channels);

//FBXM::Matrix FbxMatrixIdentity();
inline FBXM::Matrix  FbxMatrixRotX(float ang) {return FbxMatrixRotate<1,2>(ang);};
inline FBXM::Matrix  FbxMatrixRotY(float ang){return FbxMatrixRotate<0,2>(ang);};
inline FBXM::Matrix  FbxMatrixRotZ(float ang){return FbxMatrixRotate<0,1>(ang);};

inline FBXM::Vector3 FBXM::Min(const FBXM::Vector3& a, const FBXM::Vector3& b){    return FBXM::Vector3( min(a[0],b[0]),min(a[1],b[1]),min(a[2],b[2]) );}
inline FBXM::Vector3 FBXM::Max(const FBXM::Vector3& a, const FBXM::Vector3& b){    return FBXM::Vector3( max(a[0],b[0]),max(a[1],b[1]),max(a[2],b[2]) );}

FBXM::Matrix MatrixRot_pZ_mY_pX(const FBXM::Vector3& ang);
FBXM::Matrix   MatrixTranslate(const FBXM::Vector3 trans);
FBXM::Matrix   MatrixScale(const FBXM::Vector3   scale);
FBXM::Matrix MatrixSrt(FBXM::Vector3 scale, FBXM::Vector3 rotate, FBXM::Vector3 translate);
FBXM::Matrix MatrixLookAt(FBXM::Vector3 pos,FBXM::Vector3 fwd,FBXM::Vector3 up);
#endif // FBXMATH_H
