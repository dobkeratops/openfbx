#include "fbxmath.h"

typedef FbxMath::Matrix FbxMatrix;
auto    FbxMatrix::Rot_pZ_mY_pX(const Vector3& ang)->Matrix { return   Rotate<2>(ang[2])*Rotate<1>(-ang[1]) *Rotate<0>(ang[0]);  }
auto    FbxMatrix::Translate(const Vector3 trans)->Matrix { auto m=Identity(); m[3] = Vector4(trans,1.f);    return  m;}
auto    FbxMatrix::Scale(const Vector3   scale)->Matrix { auto m=Identity();for (int i=0; i<3; i++) { m[i][i]=scale[i]; };  return  m;}
auto    FbxMatrix::Srt(Vector3 scale, Vector3 rotate, Vector3 translate)->Matrix   {   return  Translate(translate) * Rot_pZ_mY_pX(rotate) * Scale(scale);   }

void    FbxMatrix::SetSRT(const float* ch)
{
    float rf=(2.f*M_PI/360.f);
    *this=Srt(
            Vector3(  ch[Transform_S_X],ch[Transform_S_Y],ch[Transform_S_Z]),
            Vector3(  ch[Transform_R_X],ch[Transform_R_Y],ch[Transform_R_Z]),
            Vector3(  ch[Transform_T_X],ch[Transform_T_Y],ch[Transform_T_Z])
            );
}

auto FbxMatrix::LookAt(Vector3 pos,Vector3 fwd,Vector3 up)->Matrix {
    auto ax=(up^fwd).normalized();
    auto az=(fwd).normalized();
    auto ay=(fwd^ax).normalized();
    return Matrix(ax,ay,az,pos);
}

