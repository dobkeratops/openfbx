#include "fbxmath.h"


FBXM::Matrix    MatrixRot_pZ_mY_pX(const FBXM::Vector3& ang) { return   FbxMatrixRotZ(ang[2])*FbxMatrixRotY(-ang[1]) *FbxMatrixRotX(ang[0]);  }
FBXM::Matrix    MatrixTranslate(const FBXM::Vector3 trans) { auto m=FbxMatrixIdentity(); m[3] = FBXM::Vector4(trans,1.f);    return  m;}
FBXM::Matrix    MatrixScale(const FBXM::Vector3   scale) { auto m=FbxMatrixIdentity();for (int i=0; i<3; i++) { m[i][i]=scale[i]; };  return  m;}
FBXM::Matrix    MatrixSrt(FBXM::Vector3 scale, FBXM::Vector3 rotate, FBXM::Vector3 translate)   {   return  MatrixTranslate(translate) *  MatrixRot_pZ_mY_pX(rotate) * MatrixScale(scale);   }

void    FbxMath::EvalSRT(Matrix* dst, const float* ch)
{
    float rf=(2.f*M_PI/360.f);
    *dst=MatrixSrt(
            Vector3(  ch[Transform_S_X],ch[Transform_S_Y],ch[Transform_S_Z]),
            Vector3(  ch[Transform_R_X],ch[Transform_R_Y],ch[Transform_R_Z]),
            Vector3(  ch[Transform_T_X],ch[Transform_T_Y],ch[Transform_T_Z])
            );
}

FBXM::Matrix MatrixLookAt(FBXM::Vector3 pos,FBXM::Vector3 fwd,FBXM::Vector3 up){
    auto ax=normalize(up^fwd);
    auto az=normalize(fwd);
    auto ay=normalize(fwd^ax);
    return FBXM::Matrix(ax,ay,az,pos);
}

