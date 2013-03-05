#ifndef fbxviewer_h
#define fbxviewer_h

#include "fbxscene.h"
#include "loadfbx.h"
/*
 * Debug rendering of loaded FBX data using fixed-pipeline glut
 */

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

class   FbxViewer : FbxScene {
public:
    static void	glVertex(const Vertex& v);
    static void	glVertex(const Vector4& v);
    static void	MatrixDraw(const Matrix& mat, float s);
    static Vector3 PermuteVertex(const Vector3& v);
    static void JointDraw(const Matrix& parentMat, const Matrix& mat,float s);
    static void DrawPoint(const Vector3& c, float s);
    static void	DrawPoint(const Vector4& c, float s);
    static void	MeshDraw(const FbxScene* scn, const Model* mdl, const FbxMesh*msh, const Matrix& mat);

    static void	ModelDrawMeshes(const FbxScene* scn, const Matrix& parentMat, const Model* mdl);
    static void	ModelDrawJoints(const FbxScene* scn, const Matrix& parentMat, const Model* mdl);
    static void DrawCubePoints(float f);
    static void	SceneDraw(const FbxScene* scn,int take, float t);
    static void	Keyboard(unsigned char key, int, int);
    static void Render();
    static void Idle();
    static int	s_PermuteSrt;
    static FbxScene*	s_pFbxScene;
    static Vector3	s_Axis;
};

#endif
