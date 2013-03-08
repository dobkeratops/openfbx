#include "fbxscene.h"


void	FbxScene::PostLoadingSetup()
{
	int	i;

    for (auto& mdl : this->allModels) {
        if (mdl->mesh) mdl->mesh->NormalizeWeightMap();
    }
    for (auto& take :this->takes)
        take->Setup();
    this->extents = Extents3();
    for (auto pModel: this->rootModels)
        this->UpdateExtents(this->extents,pModel,Matrix::Identity());

}
float FbxScene::Radius()const
{
    auto size=extents.max-extents.min;
    return sqrt(size|size)*0.5f;
}
FBXM::Vector3 FbxScene::Centre() const{
    return (extents.min+extents.max)*0.5f;
}

float FbxScene::Model::GetChannel(Channel_t c) const{
	int ci=c % 3;
	int cn=c / 3;
	const Vector3* src=(cn==0)?&this->localTranslate:(cn==1)?&this->localRotate:&this->localScale;
	return	 (*src)[ci];
}
void
FbxScene::Model::CalcLocalMatrixFromSRT()
{
    this->localMatrix = Matrix::Srt(this->localScale, this->localRotate, this->localTranslate);
}

FbxScene::Matrix
FbxScene::Model::GetGlobalMatrix() const {
    if (!parent) return this->localMatrix;
    else
        return this->parent->GetGlobalMatrix() * this->localMatrix;
}

FbxScene::Mesh*	FbxScene::CreateMeshForModel(Model*	mdl)
{
    //mdl->meshId = this->meshes.size();
    //auto mesh=fbxAppend(this->meshes,1);
    //return mesh;
    if (!mdl->mesh)
        mdl->mesh= fbxMakeUnique<Mesh>();
    return mdl->mesh.get();
}

void
FbxScene::UpdateExtents(Extents3& dst,const Model* mdl, const Matrix& parentMat)
{
    auto worldMat=parentMat* mdl->localMatrix;
    auto msh=this->GetMeshOfModel(mdl);
    if (msh){
        for (auto v:msh->Vertices) {
            auto pos=worldMat * Vector4(v,1.f);
            dst.include(Vector3(pos[0],pos[1],pos[2]));
        }
    }
    for (auto subMdl:mdl->childModels)
        UpdateExtents(dst,subMdl,worldMat);
}

FbxMath::Vector4	FbxScene::s_WeightMapColors[4]= {
    fbxvec4(1.f,0.0f,0.5f,1.f),
    fbxvec4(0.5f,1.f,0.f,1.f),
    fbxvec4(0.f,0.5f,1.f,1.f),
    fbxvec4(0.f,1.f,0.5f,1.f),
};

FbxScene::Model::Model()
{ 	weightMapColor=Vector4(0.f,0.f,0.f,0.f);
	static int r;
	r++;
    weightMapColor=s_WeightMapColors[r&3];
	this->isDeformer=false;
//	meshId=-1;
    mesh=0;
	parent=0;
}

FbxScene::PropertyDef<FbxScene::Model>	FbxScene::Model::s_Properties[]={
    {"Lcl Translation", 3,(ModelFloatPtr_t)&FbxScene::Model::localTranslate},
    {"Lcl Rotation", 3,(ModelFloatPtr_t)&FbxScene::Model::localRotate},
    {"Lcl Scaling", 3,(ModelFloatPtr_t)&FbxScene::Model::localScale},
	{0,0,0}
};


const char* FbxScene::s_ChannelNames[]=
{
	"Transform_T_X",
	"Transform_T_Y",
	"Transform_T_Z",
	"Transform_R_X",
	"Transform_R_Y",
	"Transform_R_Z",
	"Transform_S_X",
	"Transform_S_Y",
	"Transform_S_Z",
	0,
};

FbxScene::Channel_t FbxScene::GetChannelIndex(const char* name) {
	const char** src;
	int	id=0;
    for (src=s_ChannelNames; *src; src++,id++)
		if (!strcmp(*src, name))
			return	(Channel_t)id;
	return	(Channel_t)-1;
}


FbxScene::Matrix	FbxScene::Model::GetLocalMatrix()  const
{
	return	this->localMatrix;
}
FbxScene::Matrix	FbxScene::Model::GetLocalMatrixPermuteTest(int permute)  const
{
	int	n=permute;
    FbxScene::Matrix	rot;
    FbxScene::Matrix rotx=Matrix::Rotate<0>(((n&1)?1.f:-1.f)*this->localRotate[0]);
    FbxScene::Matrix roty=Matrix::Rotate<1>(((n&2)?1.f:-1.f)*this->localRotate[1]);
    FbxScene::Matrix rotz=Matrix::Rotate<2>(((n&4)?1.f:-1.f)*this->localRotate[2]);
    FbxScene::Matrix trans=Matrix::Translate(this->localTranslate);
    FbxScene::Matrix scale=Matrix::Scale(this->localScale);

	switch (n>>3) {
	case 0:rot=	roty*rotz*rotx;break;
	case 1:rot=	roty*rotx*rotz;break;
	case 2:rot=	rotz*roty*rotx;break;
	case 3:rot=	rotz*rotx*roty;break;
	case 4:rot=rotx*rotz*roty;break;
	default:rot=rotx*roty*rotz;break;
	}

	switch ((n>>6) & 7) {
	default:	return trans * rot * scale;
	case 1:		return	trans * scale * rot;
	case 2:		return	rot * trans * scale;
	case 3:		return	rot * scale * trans;
	case 4:		return	scale * rot * trans;
	case 5:		return	scale * trans * rot;
	}
}

void
FbxScene::Take::Setup(){
    this->maxt=0.f;
    for (auto& cv:curves)
        if (cv.points.size())
            this->maxt=max(this->maxt,cv.points.end()[-1].t);
}

void
FbxScene::EvalTake(CycleEvalBuffer& dst,  const FbxScene::Take* src, float t) const
{
    int	i;
    for (i=0; i<src->curves.size(); i++) {
        EvalFCurve(dst, &src->curves[i],t);
    }
}

void
FbxScene::EvalFCurve(CycleEvalBuffer& dst, const FbxScene::FCurve* cv, float t) const
{
    float v=0.f;
    auto p=cv->points.begin(),p1=p+1;
    if (cv->points.size()<=1) {
        v=cv->points[0].value;
    }else{
        for (;(p+1)<cv->points.end(); p++,p1++){
            if (p->t<=t && t<=p1->t) {
                break;
            }
        }
    // just get the value at t=0
    //auto v=cv->points[0].value;
        auto f=(t-p->t)/(p1->t-p->t);
        v=f*(p1->value-p->value)+p->value;
    }
    float DtoR=(2.f*M_PI/360.f);
    if (cv->channelIndex==Transform_R_X || cv->channelIndex==Transform_R_Y || cv->channelIndex==Transform_R_Z )
        v*=DtoR;
    dst[cv->modelIndex][cv->channelIndex]=v;

/*
    auto mdl=this->allModels[cv->modelIndex];
    switch (cv->channelIndex) {
    case Transform_R_X:	mdl->localRotate[0] = v;	break;
    case Transform_R_Y:	mdl->localRotate[1] = v;	break;
    case Transform_R_Z:	mdl->localRotate[2] = v;	break;
    case Transform_S_X:	mdl->localScale[0] = v;	break;
    case Transform_S_Y:	mdl->localScale[1] = v;	break;
    case Transform_S_Z:	mdl->localScale[2] = v;	break;
    case Transform_T_X:	mdl->localTranslate[0] = v;	break;
    case Transform_T_Y:	mdl->localTranslate[1] = v;	break;
    case Transform_T_Z:	mdl->localTranslate[2] = v;	break;

    }
*/
};

int	g_permute=0;
void
FbxScene::EvalMatrixArray(Matrix* dst, const CycleEvalBuffer* src) const
{
    int	num=allModels.size();
    // hack, try to recompute basebose..
    if (0)
    for(int i=0; i<num; i++) {
        auto mdl = this->allModels[i];
        mdl->CalcLocalMatrixFromSRT();
    }

    std::vector<Matrix>	lcl;
    lcl.resize (num);
    Matrix	origin=Matrix::Identity();

    int	i;
    for (i=0; i<num; i++) {
        EvalSRT(&lcl[i], &(*src)[i][0]);
    }
    // todo: root model list
    for (i=0; i<num ;i++) {
        dst[i]=lcl[i];
    }
    // multiply out ..
    for (auto& ln : hrcLinks) {
        if(ln.child>=0 && ln.parent>=0) {
            dst[ln.child] = dst[ln.parent] * dst[ln.child];
        }
    }
    dst[i]=lcl[i];
    g_permute++;

}

void
FbxScene::Mesh::PostLoadingSetup() {
    int triStart=0;
    for (int i=0; i<PolygonVertexIndex.size();i++)
    {
        int pvi=PolygonVertexIndex[i]; if (pvi<0) pvi=~pvi;
        if ((i-triStart)>=2) {
            triangles.emplace_back(PolygonVertexIndex[triStart],PolygonVertexIndex[i-1],pvi);
        }
        if (PolygonVertexIndex[i]<0){
            triStart = i+1;
        }
    }
    MakeRenderable();
}

void
FbxScene::VertexBoneWeights::add(int bi, float w)
{
    if (this->num<MAX) { this->boneIndex[this->num]=bi; this->boneWeight[this->num]=w; this->num++;}
    else {
        // find lowest and replace.
        int	i,wmin=1000000.f,imin=0;
        for (i=0; i<MAX; i++) {
            if (this->boneWeight[i]<wmin)  {wmin=this->boneWeight[i];imin=i;}
        }
        this->boneWeight[imin]=w;
        this->boneIndex[imin]=bi;

    }
}


void
FbxDumpModel(const FbxScene* scn, const FbxScene::Model* mdl, const FbxScene::Matrix& parent,int depth,IWriter* out)
{
    FbxScene::Matrix 	lmat=parent * mdl->GetLocalMatrix();
    out->beginMap();
    out->keyValue("name", mdl->name);
    out->keyValue("globalMatrix", lmat);
    out->endMap();
    int	i;

    for (auto& sm: mdl->childModels)
        FbxDumpModel(scn,	sm, lmat,depth+1, out);
}

void
FbxDumpScene(const FbxScene* scn,IWriter* out)
{
    out->beginMap();
    out->keyValue("min",scn->extents.min);
    out->keyValue("max",scn->extents.max);
    auto    ident=FBXM::Matrix::Identity();
    out->keyValue("numRootModels",(int)scn->rootModels.size());
    out->beginKeyValue("modelNames");
    out->beginArray(scn->allModels.size());
    int	i;
    for (i=0; i<scn->allModels.size(); i++) {
        out->value(scn->allModels[i]->name);
    }
    out->endArray();
    out->endKeyValue();

    {
        out->beginKeyValue("textureFileNames");
        out->beginArray(scn->textures.size());
        for (auto tx:scn->textures) {
           out->value(tx.filename);
        }
        out->endArray();
        out->endKeyValue();
    }

    for (auto& mp :scn->rootModels)
        FbxDumpModel(scn, mp, ident, 0, out);

    out->endMap();
}

/*Write out what we read in...*/
void	FbxScene::Triangle::Write(IWriter* out) const {
    out->beginArray(3);
    out->value(this->vertex[0]);
    out->value(this->vertex[1]);
    out->value(this->vertex[2]);
    out->endArray();
}

void	FbxScene::Quad::Write(IWriter* out) const{
    out->beginArray(4);
    out->value((*this)[0]);
    out->value((*this)[1]);
    out->value((*this)[2]);
    out->value((*this)[3]);
    out->endArray();
}

void
FbxScene::InitCycleEvalBuffer(CycleEvalBuffer& dst)  const
{
    dst.resize(this->allModels.size());
    float f;

    // Clear by taking the rest-pose values

    for (int i=0; i<this->allModels.size(); i++) {
        auto mdl=this->allModels[i];
        auto& d=dst[i];
        for (int ci=0; ci<NumChannels; ci++) {
            d[ci]=mdl->GetChannel((Channel_t)ci);
        }
    }
}

void
FbxScene::Mesh::MakeRenderable()
{
    // Find every unique combination of position,texture coordinates & other surface attributes
    // these are 'RenderVertices'
    struct RenderVertexTmp {
        RenderVertexTmp* next=0;
        int outputIndex=-1;
        int pos;
        array<Vector2,MaxUVLayers> tex;
        RenderVertexTmp(){
            for (auto& t:this->tex)
            {t[0]=0.f;t[1]=0.f;}
        }
        bool operator==(const RenderVertexTmp& other) const
        {
            int i;
            if (this->pos!=other.pos) return false;
            for (i=0; i<tex.size();i++)
                if (this->tex[i]!=other.tex[i])
                    return false;
            return true;
        }
    };

    // create unique polygon-vertex array.
    vector<RenderVertexTmp*> rvtPerVertex;
    vector<RenderVertexTmp*> output;
    rvtPerVertex.resize(this->Vertices.size());
    fill_n(rvtPerVertex.begin(),this->Vertices.size(),nullptr);

    ASSERT(this->LayerElementUVs.size()<MaxUVLayers && "uv layers exceeded");

    // [1] Create polygon render vertices
    vector<RenderVertexTmp> polygonRenderVertex;
    polygonRenderVertex.resize(this->PolygonVertexIndex.size());

    size_t i,layer;
    for (i=0; i<PolygonVertexIndex.size();i++) {
        auto pvi=PolygonVertexIndex[i];
        if (pvi<0) pvi=~pvi;
        polygonRenderVertex[i].pos= pvi;
    }

    for (layer=0; layer<this->LayerElementUVs.size(); layer++) {
        auto &layeruv=this->LayerElementUVs[layer];
        ASSERT(layeruv.UVIndex.size()==this->PolygonVertexIndex.size() &&"assumption about layout")
                for (i=0; i<layeruv.UVIndex.size(); i++) {
            auto id=layeruv.UVIndex[i];
            polygonRenderVertex[i].tex[layer]=layeruv.UV[id];
        }
    }

    // [2] find unique ones & make translation table
    vector<int> renderVertexIndex;
    renderVertexIndex.resize(this->PolygonVertexIndex.size());
    fill_n(renderVertexIndex.begin(),renderVertexIndex.size(),-1);

    if (false) {
        // debug: no reduction
        output.resize(this->PolygonVertexIndex.size());
        for (i=0; i<this->PolygonVertexIndex.size();i++){
            renderVertexIndex[i]=i;
            output[i]=&polygonRenderVertex[i];
            output[i]->outputIndex=i;
        }
    }else {
        // scan & reduce unique rendervertices;
        for (i=0; i<this->PolygonVertexIndex.size(); i++)
        {
            auto &prv=polygonRenderVertex[i];
            auto pSrc=&rvtPerVertex[prv.pos];
            auto src=*pSrc;

            // we need the index of this element..
            for (; src;src=src->next) {
                if (*src==prv) {
                    break;
                }
            }
            if (!src) {
                src=&prv; // this poly vertex added to output
                src->outputIndex = output.size();
                output.push_back(src);
                // add to list of poly-vertices on the pos-vertex
                src->next=*pSrc;*pSrc=src;
            }
            renderVertexIndex[i]=src->outputIndex;
        }
    }
    // emit 'render-vertices'
    this->renderVertex.resize(output.size());
    for (i=0; i<output.size();i++) {
        auto&src=*output[i];
        auto&dst=this->renderVertex[i];
        dst.posIndex=src.pos;
        int i;
        for (i=0; i<MaxUVLayers;i++)
            dst.texcoord[i]=src.tex[i];
        dst.normal=Vector3(0.f,0.f,1.f);
        dst.color=0xffffffff;
    }


    // emit triangles
    for (i=2; i<this->PolygonVertexIndex.size();){
        int i0=i-2;
        for (;i<this->PolygonVertexIndex.size();i++) {
            int pv0=i0;
            int pv1=i-1;
            int pv2=i;
            auto v0=renderVertexIndex[pv0];
            auto v1=renderVertexIndex[pv1];
            auto v2=renderVertexIndex[pv2];
            this->renderTriangles.emplace_back(v0,v1,v2);
            if (this->PolygonVertexIndex[i]<0)
                break;
        }
        i+=3;
    }
    #ifdef DEBUG
        printf("raw tris=%d render tris=%d\n",this->triangles.size(),this->renderTriangles.size());
        printf("PositionVertices=%d\nRenderVertices=%d\nPolygonVertices=%d\n",
               this->Vertices.size(),this->renderVertex.size(),this->PolygonVertexIndex.size());
    // assert..
        for (int    i=0; i<this->PolygonVertexIndex.size(); i++)
        {
            int pvi=this->PolygonVertexIndex[i]; if (pvi<0) pvi=~pvi;
            int rvi=renderVertexIndex[i];
            auto &rv=this->renderVertex[rvi];
            printf("pvi[%d],rvi[%d] pvPos=%d  rvPos=%d\n",i, renderVertexIndex[i], pvi,rv.posIndex);
        }
        printf("render vertex creation done");
    #endif
}


FbxScene::Model*
FbxScene::GetModel(const char* mdlName)
{

    for(auto mp :allModels)
    { if (mp->name==mdlName) return mp;
    }
    return  nullptr;
}

int
FbxScene::GetIndexOfModel(const char* mdlName)
{
    int i=0;
    for (auto pm :allModels)
    {	if (pm->name==mdlName) return i;
        i++;
    }
    return  -1;
}

void
FbxScene::Mesh::NormalizeWeightMap() {
    //FOR_EACH_IN (wm,this->weightMap)
    for (size_t n=0; n<this->vertexWeightMap.size(); n++)
    {
        auto &wm=this->vertexWeightMap[n];
        float sumw=0.f; int i; for (i=0; i<wm.num; i++) sumw+=wm.boneWeight[i];
        if (sumw>0.f)
            for (i=0; i<wm.num; i++) wm.boneWeight[i]/=sumw;
    }
}



