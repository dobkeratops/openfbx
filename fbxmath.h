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
    typedef std::array<float,2> Vector2;
    typedef std::array<float,3> Vector3;
    typedef std::array<float,4> Vector4;
    typedef std::array<Vector4,4> Matrix;
    typedef std::array<Vector3,2> Extents;
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

inline FBXM::Vector3 FbxVector3(float x,float y,float z) {return FBXM::Vector3({{x,y,z}});}
using namespace std;
template<typename T> inline array<T,4> concat(const array<T,3>& abc, const T&d) { return {{abc[0],abc[1],abc[2],d}};}

inline FBXM::Vector3 operator*(const FBXM::Vector3& a,float f)  { return FbxVector3(a[0]*f,a[1]*f,a[2]*f); }
inline FBXM::Vector3 operator+(const FBXM::Vector3& a,const FBXM::Vector3& b)  { return {{a[0]+b[0], a[1]+b[1], a[2]+b[2]}}; }
inline FBXM::Vector3 operator-(const FBXM::Vector3& a,const FBXM::Vector3& b)  { return {{a[0]-b[0], a[1]-b[1], a[2]-b[2]}}; }
inline FBXM::Vector3& operator*=(FBXM::Vector3& a,float f)  { a[0]=a[0]*f; a[1]=a[1]*f; a[2]=a[2]*f; return a; }
inline FBXM::Vector4 operator*(const FBXM::Vector4& a,float f)  { return {{a[0]*f,a[1]*f,a[2]*f,a[3]*f}}; }
inline FBXM::Vector4 operator+(const FBXM::Vector4& a,const FBXM::Vector4& b)  { return {{a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3]}}; }
inline FBXM::Vector4 operator-(const FBXM::Vector4& a,const FBXM::Vector4& b)  { return {{a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3]}}; }
inline FBXM::Vector4 operator+=(FBXM::Vector4& a,const FBXM::Vector4& b)  { a[0]+=b[0];a[1]+=b[1]; a[2]+=b[2]; a[3]+=b[3]; return a; }

inline FBXM::Vector4 operator*(const FBXM::Matrix& a,const FBXM::Vector4& b)  { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; };
inline FBXM::Matrix  operator*(const FBXM::Matrix& m,const FBXM::Matrix& b)  { return {{m * b[0], m* b[1], m * b[2], m * b[3]}};}
inline FBXM::Matrix FbxMatrixIdentity() {
    return {{
        {{1.f,0.f,0.f,0.f}},
        {{0.f,1.f,0.f,0.f}},
        {{0.f,0.f,1.f,0.f}},
        {{0.f,0.f,0.f,1.f}} }};
}
inline FBXM::Vector3	operator^(const FBXM::Vector3& a,const FBXM::Vector3& b){return	{{a[1]*b[2]-a[2]*b[1],	a[2]*b[0]-a[0]*b[2],	a[0]*b[1]-a[1]*b[0]}};}
inline float			operator|(const FBXM::Vector4& a,const FBXM::Vector4& b) {return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];}
inline float			operator|(const FBXM::Vector3& a,const FBXM::Vector3& b) {return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
inline FBXM::Vector3	FbxNormalize(const FBXM::Vector3& a) {return a*(1.f/sqrt(a|a));}

inline FBXM::Vector3 FbxVector3(float f=0.f) {return {{f,f,f}};}
inline FBXM::Vector4 FbxVector4(float f=0.f) {return {{f,f,f,f}};}
inline FBXM::Vector4 FbxAxisX (float fval=1.f) {return {{fval,0.f,0.f,0.f}};}
inline FBXM::Vector4 FbxAxisY (float fval=1.f) {return {{0.f,fval,0.f,0.f}};}
inline FBXM::Vector4 FbxAxisZ (float fval=1.f) {return {{0.f,0.f,fval,0.f}};}
inline FBXM::Vector4 FbxAxisW (float fval=1.f) {return {{0.f,0.f,0.f,fval}};}
inline FBXM::Vector3 FbxMin(const FBXM::Vector3& a, const FBXM::Vector3& b){    return {{ min(a[0],b[0]),min(a[1],b[1]),min(a[2],b[2]) }};}
inline FBXM::Vector3 FbxMax(const FBXM::Vector3& a, const FBXM::Vector3 & b){    return {{ max(a[0],b[0]),max(a[1],b[1]),max(a[2],b[2]) }};}

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

FBXM::Matrix FbxMatrixRot_pZ_mY_pX(const FBXM::Vector3& ang);
FBXM::Matrix   FbxMatrixTranslate(const FBXM::Vector3 trans);
FBXM::Matrix   FbxMatrixScale(const FBXM::Vector3   scale);
FBXM::Matrix FbxMatrixSrt(FBXM::Vector3 scale, FBXM::Vector3 rotate, FBXM::Vector3 translate);
FBXM::Matrix FbxMatrixLookAt(FBXM::Vector3 pos,FBXM::Vector3 fwd,FBXM::Vector3 up);
inline FBXM::Extents FbxExtentsInit() { return {{ {{FLT_MAX,FLT_MAX,FLT_MAX}},{{-FLT_MAX,-FLT_MAX,-FLT_MAX}} }}; };
inline void FbxExtentsInclude(FBXM::Extents& dst, FBXM::Vector3 vec) { dst[0]=FbxMin(dst[0],vec); dst[1]=FbxMax(dst[1],vec); }
#endif // FBXMATH_H
