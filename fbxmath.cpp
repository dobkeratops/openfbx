#include "fbxmath.h"


FBXM::Matrix    FBXM::Matrix::Rot_pZ_mY_pX(const FBXM::Vector3& ang) { return   Rotate<2>(ang[2])*Rotate<1>(-ang[1]) *Rotate<0>(ang[0]);  }
FBXM::Matrix    FBXM::Matrix::Translate(const FBXM::Vector3 trans) { auto m=Identity(); m[3] = FBXM::Vector4(trans,1.f);    return  m;}
FBXM::Matrix    FBXM::Matrix::Scale(const FBXM::Vector3   scale) { auto m=Identity();for (int i=0; i<3; i++) { m[i][i]=scale[i]; };  return  m;}
FBXM::Matrix    FBXM::Matrix::Srt(Vector3 scale, Vector3 rotate, Vector3 translate)   {   return  Translate(translate) * Rot_pZ_mY_pX(rotate) * Scale(scale);   }

void    FbxMath::EvalSRT(Matrix* dst, const float* ch)
{
    float rf=(2.f*M_PI/360.f);
    *dst=Matrix::Srt(
            Vector3(  ch[Transform_S_X],ch[Transform_S_Y],ch[Transform_S_Z]),
            Vector3(  ch[Transform_R_X],ch[Transform_R_Y],ch[Transform_R_Z]),
            Vector3(  ch[Transform_T_X],ch[Transform_T_Y],ch[Transform_T_Z])
            );
}

FBXM::Matrix FBXM::Matrix::LookAt(Vector3 pos,Vector3 fwd,Vector3 up){
    auto ax=normalize(up^fwd);
    auto az=normalize(fwd);
    auto ay=normalize(fwd^ax);
    return FBXM::Matrix(ax,ay,az,pos);
}

