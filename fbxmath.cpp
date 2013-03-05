#include "fbxmath.h"


FBXM::Matrix FbxMatrixRot_pZ_mY_pX(const FBXM::Vector3& ang) { return   FbxMatrixRotZ(ang[2])*FbxMatrixRotY(-ang[1]) *FbxMatrixRotX(ang[0]);  }
FBXM::Matrix   FbxMatrixTranslate(const FBXM::Vector3 trans) { auto m=FbxMatrixIdentity(); m[3] = concat(trans,1.f);    return  m;}
FBXM::Matrix   FbxMatrixScale(const FBXM::Vector3   scale) { auto m=FbxMatrixIdentity();for (int i=0; i<3; i++) { m[i][i]=scale[i]; };  return  m;}
FBXM::Matrix FbxMatrixSrt(FBXM::Vector3 scale, FBXM::Vector3 rotate, FBXM::Vector3 translate)   {   return  FbxMatrixTranslate(translate) *  FbxMatrixRot_pZ_mY_pX(rotate) * FbxMatrixScale(scale);   }

void    FbxMath::EvalSRT(Matrix* dst, const float* ch)
{
    float rf=(2.f*M_PI/360.f);
    *dst=FbxMatrixSrt(
            Vector3(  {ch[Transform_S_X],ch[Transform_S_Y],ch[Transform_S_Z]}),
            Vector3(  {ch[Transform_R_X],ch[Transform_R_Y],ch[Transform_R_Z]}),
            Vector3(  {ch[Transform_T_X],ch[Transform_T_Y],ch[Transform_T_Z]})
            );
}

FBXM::Matrix FbxMatrixLookAt(FBXM::Vector3 pos,FBXM::Vector3 fwd,FBXM::Vector3 up){
    auto ax=FbxNormalize(up^fwd);
    auto az=FbxNormalize(fwd);
    auto ay=FbxNormalize(fwd^ax);
    return  {concat(ax,0.f),concat(ay,0.f),concat(az,0.f),concat(pos,1.f)};
}

