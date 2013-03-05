#include "fbxviewer.h"
using namespace std;

int	FbxViewer::s_PermuteSrt=21;

void	FbxViewer::glVertex(const Vertex& v)
{
	glVertex3f( v[0],v[1],v[2]);

}
void	FbxViewer::glVertex(const Vector4& v)
{
	glVertex4f( v[0],v[1],v[2],v[3]);

}
void	FbxViewer::MatrixDraw(const Matrix& mat, float s)
{
    glBegin(GL_LINES);
    glColor3f(1.f,0.f,0.f);
    glVertex( mat[3]);
    glVertex( mat[3] + mat[0]*s);
    glColor3f(0.f,1.f,0.f);
    glVertex( mat[3]);
    glVertex( mat[3] + mat[1]*s);
    glColor3f(0.f,0.f,1.f);
    glVertex( mat[3]);
    glVertex( mat[3] + mat[2]*s);
    glEnd();
}

FBXM::Vector3 FbxViewer::PermuteVertex(const Vector3& v)
{
	return	v;
}
void	FbxViewer::JointDraw(const Matrix& parentMat, const Matrix& mat,float s)
{
    glBegin(GL_LINES);

    auto axisZ = mat[3]-parentMat[3];

    glColor3f(0.25f, 0.75f,0.75f);
    glVertex( parentMat[3]);
    glVertex( mat[3]);

    glEnd();
    MatrixDraw(mat,s);
}


void	FbxViewer::DrawPoint(const Vector3& c, float s) {
	glBegin(GL_LINES);
	glColor3f(0.25f, 0.75f,0.75f);

    glVertex( c+Vector3({-s,0,0}) );
    glVertex( c+Vector3({s,0,0}) );
    glVertex( c+Vector3({0,-s,0}) );
    glVertex( c+Vector3({0,s,0}) );
    glVertex( c+Vector3({0,0,-s}) );
    glVertex( c+Vector3({0,0,s}) );
	glEnd();
} 

void	FbxViewer::DrawPoint(const Vector4& c, float s) {
    DrawPoint(Vector3({c[0],c[1],c[2]}),s);
}

void	FbxViewer::MeshDraw(const FbxScene* scn, const Model* mdl, const FbxMesh*msh, const Matrix& mat)
{
	int	i;
	glBegin(GL_TRIANGLES);
	glColor3f(0.5f,0.5f,0.5f);

	for (auto& tri : msh->triangles) {
		int	k;
		for (k=0; k<3; k++)
		{
			int	vi=tri.at(k);
            Vector4	weightColor={{0.f,0.f,0.f,0.f}};
			int	bii;
			for (bii=0; bii<msh->weightMap[vi].size(); bii++) {
				int	bi=msh->weightMap[vi].boneIndex[bii];;
				weightColor+=scn->allModels[bi]->weightMapColor * msh->weightMap[vi].boneWeight[bii];
			}
			glColor3f(weightColor[0],weightColor[1],weightColor[2]);
			glVertex( mat * concat(PermuteVertex(msh->vertices[vi]),1.f));
		}
	}
	glEnd();

	glBegin(GL_POINTS);
	glColor3f(1.f,1.f,1.f);

	for (auto& v: msh->vertices) {
		glVertex(mat *concat(PermuteVertex(v),1.f));
	}
	glEnd();

}

void	FbxViewer::ModelDrawMeshes(const FbxScene* scn, const Matrix& parentMat, const Model* mdl)
{
	auto lmat=mdl->GetLocalMatrix();
    auto  mat = parentMat*mdl->GetLocalMatrix() ;

	auto msh=scn->GetMeshOfModel(mdl);
	if (msh)
        MeshDraw(scn, mdl,msh, mat);

    for(auto& subMdl : mdl->childModels) ModelDrawMeshes(scn, mat, subMdl);

}
void	FbxViewer::ModelDrawJoints(const FbxScene* scn, const Matrix& parentMat, const Model* mdl)
{
	auto lmat=mdl->GetLocalMatrix();
    auto  mat = parentMat*mdl->GetLocalMatrix() ;
    JointDraw(parentMat,mat,  0.05f);

	
    for( auto& subMdl : mdl->childModels) ModelDrawJoints(scn, mat, subMdl);
}


void
FbxViewer::DrawCubePoints(float f)
{
	float s;
	for (int i=0; i<8; i++) 
	{
        auto pos=FBXM::Vector3 ({(i&1)?f:-f, (i&2)?f:-f, (i&4)?f:-f}) ;
        DrawPoint( pos, f*0.05);
	}
}

void	FbxViewer::SceneDraw(const FbxScene* scn,int take,float t)
{
    auto    mat=FbxMatrixIdentity();
	DrawCubePoints(1.0);

	glEnable (GL_DEPTH_TEST);
	for (auto& mp :scn->rootModels)
        ModelDrawMeshes(scn, mat, mp);

	glDisable (GL_DEPTH_TEST);

	for (auto& mp :scn->rootModels)
        ModelDrawJoints(scn, mat, mp);

	glEnable (GL_DEPTH_TEST);

	if (scn->takes.size()) 
	{
        CycleEvalBuffer	eval;
        Matrix	matrices[256];
		scn->InitCycleEvalBuffer(eval);
        scn->EvalTake(eval,scn->takes[take], t);
		scn->EvalMatrixArray(&matrices[0], &eval);
        size_t	i;
		for (i=0; i<scn->allModels.size(); i++) {
            MatrixDraw(matrices[i], 0.025f);
		}
		for (auto& ln: scn->hrcLinks) {
            JointDraw(matrices[ln.parent], matrices[ln.child],0.025);
		}
	}

}


FbxScene*	FbxViewer::s_pFbxScene;
FbxMath::Vector3	FbxViewer::s_Axis={{0.707f,0.f,0.707f}};
void	FbxViewer::Keyboard(unsigned char key, int, int)
{
	switch (key)
	{
    case '1': s_Axis=Vector3({1.f,0.f,0.f}); break;
    case '2': s_Axis=Vector3({0.f,1.f,0.f}); break;
    case '3': s_Axis=Vector3({0.f,0.f,1.f}); break;
    case '4': s_Axis=Vector3({0.707f,0.f,.707f}); break;
    case 'q': s_PermuteSrt++; break;
    case 'a': s_PermuteSrt--; break;
    case 'w': s_PermuteSrt+=8; break;
    case 's': s_PermuteSrt-=8; break;
    case 'e': s_PermuteSrt+=64; break;
    case 'd': s_PermuteSrt-=64; break;
	}
	if (key) {
        fbx_printf("permute srt index=%d\n",s_PermuteSrt);
	}
}

void	FbxViewer::Render()
{
	//GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
	//glClearColor(0.f,0.f,0.f,0.f);
	float v=sqrt(1.f/3.f);
	//g_Hack++;
	static float angle=1.9f; angle+=0.09f;
	float angle1=angle*0.19f+0.7f;
	float angle2=angle*0.05f+0.5f;
    static float anim_t = 0.f;
    static int take=0;
    float maxt = s_pFbxScene->takes[take]->maxt;
    anim_t+=maxt/50.f;
    if (anim_t>maxt) {
        take=(take+1)%s_pFbxScene->takes.size();
        anim_t=0.f;
    }

	glPushMatrix();
//	glRotatef(angle,g_Axis[2],g_Axis[1],g_Axis[0]);
	glRotatef(-90,	1.0,	0.0,	0.0);
	glRotatef(angle,	0.0,	0.0,	1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBegin(GL_LINES);
	glColor3f(1.f,0.f,0.f);
	glVertex3f(0.f,0.f,0.f);
	glVertex3f(1.f,0.f,0.f);
	glColor3f(0.f,1.f,0.f);
	glVertex3f(-0.f,1.f,0.f);
	glVertex3f(-0.f,-0.f,0.f);
	glColor3f(0.f,0.f,1.f);
	glVertex3f(-0.f,0.f,1.f);
	glVertex3f(-0.f,0.f,0.f);
	glEnd();
    SceneDraw(s_pFbxScene,take,anim_t);
	glFlush();
	glutSwapBuffers();
	glPopMatrix();
}
void FbxViewer::Idle() {
    glutPostRedisplay();
}

#ifdef FBXVIEWER_MAIN
int main(int argc, const char** argv)
{
	std::ifstream ft; ft.open("temp.txt");
	int	p;
	char c; ft>>c; cout <<c; cout <<"\n";
	cout << ft.tellg()<<"\n";
	ft.unget();
	cout << ft.tellg() << "\n";

	ft.close();

	fbx_printf("fbx loader test:-\n");
    FbxScene	scn;
	ifstream fs;
	auto srcfile=argv[1];
	if (!srcfile) {
		srcfile="data/test.fbx";
    }
	fbx_printf("%s",srcfile);
	fs.open(srcfile,ios_base::in);
	if (!fs) {
		fbx_printf ("could not open %s",srcfile);
		return 0;
	}
    LoadFbx(&scn,fs);
    FbxViewer::s_pFbxScene=&scn;
	fs.close();
	IWriter outp(stdout);
	FbxDumpScene(&scn, &outp);

	glutInit(&argc,(char**) argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA|GLUT_MULTISAMPLE);
    auto win=glutCreateWindow("fbx load test");
	glutReshapeWindow(1024,1024);
    glutDisplayFunc(FbxViewer::Render);
    glutIdleFunc(FbxViewer::Idle);
    glutKeyboardFunc(FbxViewer::Keyboard);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.1,0.1,-0.1,0.1,0.1,5.f);
	glTranslatef(0,-1.0, -2.f*0.66);
	glMatrixMode(GL_MODELVIEW);

	glutMainLoop();
}
#endif
