#include "loadfbx.h"

using namespace std;


class	FbxHeader : public FbxString<256>
{
public:
    FbxHeader() {}
    bool	LoadNext(ifstream& src);
    FbxHeader&operator=(const char*src) { strncpy(&this->at(0),src,256); return *this;}
};

class   FbxSubBlocks {
public:
    ifstream*   pf;
    FbxSubBlocks(ifstream& file) {
        pf=&file;
        EnterBlock(*pf);
    }
    FbxHeader   hdr;
    const char* c_str() const { return hdr.c_str();}
    bool operator==(const char* txt) {return hdr==txt;}
    bool get(){
        if (pf->eof()) return false;
        return hdr.LoadNext(*pf);
    }
    ~FbxSubBlocks(){
        ExitBlock(*pf);
    }
};

void FatalError(const char* ) {
}



void
LoadFbxAnimCurves(FbxScene* scn, ifstream& src, FbxScene::Take* dst, const char* mdlName, const char* channelName)
{
    fbx_printf("load chanel %s for model %s{\n", channelName, mdlName);
	FbxHeader hdr;

	int	keyCount=-1;
    for (FbxSubBlocks hdr(src);hdr.get();)
	{
		// channel accumulates sub-channel name
		if (hdr=="Channel:") {
			FbxString<245> subn;
            ReadQuotedString(subn,src);
			std::string chn;
			chn=channelName;
			if (strlen(channelName)>0)
				chn+="_";
			chn+= subn;
            fbx_printf("sub channel %s for model %s{\n", channelName, mdlName);
			LoadFbxAnimCurves(scn, src, dst, mdlName, chn.c_str());
            fbx_printf("}%s %s \n", channelName, mdlName);
		} else
		if (hdr=="KeyCount:")
		{
			ASSERT(keyCount==-1);
			src>>keyCount;
		}
		else if (hdr=="Key:")
		{
            fbx_printf("load keys[%d]:{\n", keyCount);
            FbxScene::FCurve newCurve;
			ASSERT(keyCount>=0)
			int	i=0;
			newCurve.boneName=mdlName;
			newCurve.channelName = channelName;
			newCurve.modelIndex = scn->GetIndexOfModel(mdlName);
            newCurve.channelIndex = FbxScene::GetChannelIndex(channelName);
//			if(newCurve.modelIndex<0) {fbx_printf("model not found %s", mdlName); exit(0);}
//			if(newCurve.channelIndex<0) {fbx_printf("channel not found %s", channelName); exit(0);}

			SkipWhitespace(src);
			if (keyCount>=0 && fbxIsNumberStart(src))
			{
				do
				{
					
					char comma; double t; double f; char l;
					src >> t >> comma >> f >> comma >> l;
                    fbx_printf("channel data:\t%lf\t%lf\t%c\n", t,f,l);
                    newCurve.points.push_back(FbxScene::FCurve::tvalue(t,f));
                } while (SkipComma(src));
				newCurve.points.shrink_to_fit();
			}

			dst->curves.push_back(newCurve); // todo emplace_back

		}
	}
    fbx_printf("}\n");
//	ExitBlock(src);
}


bool
FbxHeader::LoadNext(ifstream& src)
{
	auto* hdr=this;
	char* header=&hdr->at(0);
	int	size=sizeof(*hdr);
		fbx_dumpline(src);
    *this="";
	while (!src.eof())
	{
		auto c=src.peek();
		if  (c=='}')
			return false;
		if (c=='{') SkipBlock(src);
		else if (c==';') {
			while (src.get()!='\n') {};
		}
		else
		if (c=='\"') {
			src.get(); while (src.get()!='\"') { };
		}
		else
		if (fbxIsSymbolStart(c)) {
			char* dst= &hdr->at(0);
			auto	symbolstart=src.tellg();
			do { c=src.get();
				if (dst-header<(size-1))
					*dst++=c;
				if (c==':') {
					*dst++=0;
					return true;
				}
			} while (fbxIsSymbolCont(c));
		}
		else src.get();
	}
	return	false;
};

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
FbxScene::FbxMesh::NormalizeWeightMap() {
	//FOR_EACH_IN (wm,this->weightMap)
	for (size_t n=0; n<this->weightMap.size(); n++)
	{
		auto &wm=this->weightMap[n];
		float sumw=0.f; int i; for (i=0; i<wm.num; i++) sumw+=wm.boneWeight[i];
		if (sumw>0.f)
			for (i=0; i<wm.num; i++) wm.boneWeight[i]/=sumw;
	}
}

void    fbx_printf(const FbxMath::Vector3& v) {
    fbx_printf("[%.5f %.5f %.5f]",v[0],v[1],v[2]);
}
void    fbx_printf(const FbxMath::Vector4& v) {
    fbx_printf("[%.5f %.5f %.5f %.5f]",v[0],v[1],v[2],v[3]);

}

void    fbx_printf(const FbxMath::Matrix& m) {
	int	i;
    fbx_printf("[");
	for (i=0; i<4; i++) {
	 fbx_printf(m[i]);
	}
	fbx_printf("]\n");
}
template<int N>
void	fbx_printf(const FbxString<N>& str) {
	fbx_printf(&str[0]);
}

inline void	write(IWriter* dst, const FbxScene* scn) {scn->Write(dst);};
//inline bool StrEq(const char*a ,const char* b) { return !strcmp(a,b); }
//inline bool	SkipComma(ifstream& src)
//{
//	char c; src>>c; if (c==',') return true; else src.unget(); return false;
//}



FbxScene::Vector3	LoadVector3(ifstream& src) {
	char comma;
	float x,y,z;
	src >> x  >> comma >> y >> comma >> z;
    return {{x,y,z}};
}



void    LoadFbxConnections(FbxScene* scn, ifstream& file)
{
    fbx_printf("Connections{");

    for (FbxSubBlocks hdr(file); hdr.get();)
    {
        if (hdr=="Connect:")
        {

            FbxString<128> type,from,to;
            ReadQuotedString(type,file); SkipComma(file);
            ReadQuotedString(from,file); SkipComma(file);
            ReadQuotedString(to,file);
            auto fromMdl = scn->GetModel(from);
            if (!(type=="OO"))
                continue;
            vector<FbxScene::Model*>* toMdlList;
            if (to=="Model::Scene")
                toMdlList = &scn->rootModels;
            else {
                auto toMdl = scn->GetModel(to);
                if (!toMdl) continue;   // cant' link, its not loaded.
                toMdlList =&toMdl->childModels;
				if (fromMdl)
					fromMdl->parent = toMdl;
            }
            if (fromMdl) 
			{
                toMdlList->push_back(fromMdl);
                scn->hrcLinks.push_back(FbxScene::HrcLink(scn->GetIndexOfModel(from),scn->GetIndexOfModel(to)));
			}
			// TODO: Some of these connections associate the deformers
			// we might have made assumptions on deformer connection from the names
	
            fbx_printf("Connected(%s)\t%s\t->\t%s\n", type.c_str(), from.c_str(), to.c_str());
        }
    };
    fbx_printf("Connections}");
}


void
LoadFbxTakes(FbxScene* scn, ifstream& src)
{
    for (FbxSubBlocks hdr(src); hdr.get();)
	{
        fbx_printf("takes-header:%s\n",hdr.hdr.c_str());

		if (!(hdr=="Take:")) 
			continue;
        auto* take = new FbxScene::Take;
		scn->takes.push_back(take);
        ReadQuotedString(take->name,src);
		fbx_printf("take->name %s{", take->name.c_str());
		{
            fbx_printf("1\n");
            for (FbxSubBlocks hdr(src); hdr.get();) {
				fbx_printf("takes-subheader:%s\n",hdr.c_str());
				if (hdr=="Model:") {
					fbx_printf("loading\n");
					FbxString<256>	mdlName;
                    ReadQuotedString(mdlName,src);
					fbx_printf("name=%s\n",mdlName.c_str());
					LoadFbxAnimCurves(scn, src, take, mdlName.c_str(), "");
					fbx_printf("anim curves loaded\n");
				}
				fbx_printf("takes-subheader next\n");
            }
			fbx_printf("Take}\n");
		}
        take->Setup();
		fbx_printf("}%s", take->name.c_str());
    }
	fbx_printf("debug, test parsing of LoadFbxTakes");
}



/*
void	LoadFbxTake(Fbx* scn, ifstream& src)
{
	auto	cycle = new Fbx::AnimCycle;
	scn->animCycles.push_back(cycle);
    ReadQuotedString(cycle->name, src);
	EnterBlock(src);
	for (FbxHeader hdr ;hdr.LoadNext(src); ) {

	}
	ExitBlock(src);
}
*/

/*	template<typename OBJECT>
	struct	PropertyDef { const char* propertyName; int dim; float OBJECT::*offset; };

	static	PropertyDef<Model>	s_ModelProperties[];
*/



void	LoadFbxModelProperties(FbxScene* scn, FbxScene::Model* mdl, ifstream & src)
{
    for (auto pm=FbxScene::Model::s_Properties; pm->name; pm++)
		fbx_printf("%s", pm->name);
    for (FbxSubBlocks hdr(src); hdr.get();)
	{
		if (hdr=="Property:")
		{
			FbxString<256> name, type, unknown;
            ReadQuotedString(name,src); SkipComma(src);
            ReadQuotedString(type, src);SkipComma(src);
            ReadQuotedString(unknown,src);SkipComma(src);
			if (name=="Lcl Translation") {
				mdl->localTranslate = LoadVector3(src); SkipComma(src);

			} else
			if (name=="Lcl Rotation") {
				mdl->localRotate = LoadVector3(src); SkipComma(src);
			} else
			if (name=="Lcl Scaling") {
				mdl->localScale = LoadVector3(src); SkipComma(src);
			}

			// else - parameter table for defaults.

		}
	}
	mdl->localRotate*=(2.f*M_PI/360.f);

	mdl->CalcLocalMatrixFromSRT();
	fbx_printf(mdl->localMatrix);
}

void
LoadFbxModel(FbxScene* pscn, FbxScene::Model* mdl, const char modelName[], const char modelType[], ifstream& src)
{
    mdl->name = modelName;
    FbxScene::FbxMesh*	mesh=0;
    for (FbxSubBlocks hdr(src); hdr.get();)
	{
		fbx_dumpline(src);
		if (hdr=="Properties60:") {
			LoadFbxModelProperties(pscn,mdl, src);
		}
		if (hdr=="Vertices:")
		{
			if (!mesh) mesh=pscn->CreateMeshForModel(mdl);
			int	vertexIndex=0;
			while (fbxIsNumberStart(src))
			{
                FbxScene::Vertex	v;
				v=LoadVector3(src);
				mesh->vertices.push_back( v);
				vertexIndex++;
				if (!SkipComma(src))
					break;
			}
		}
		if (hdr=="PolygonVertexIndex:")
		{
			int	triIndex=0;
			while (fbxIsNumberStart(src))
			{
				int	 pv0,pv1; char c;
				src>> pv0 >> c >> pv1 >> c;
				int	pvn;
				do {
					src >> pvn;
					int	pvnn = pvn>=0?pvn:(-pvn-1);
                    mesh->triangles.push_back( FbxScene::Triangle(pv0,pv1,pvnn) );
					pv1=pvnn;

				} while (SkipComma(src) && pvn>=0);
			}
		}
	}
}

template<typename T>
void	LoadFbxNumericArray(vector<T>*	dst, ifstream& src)
{
	if (fbxIsNumberStart(src)) {
		do {dst->push_back(Load<T>(src)); }
		while ( SkipComma(src));
		dst->shrink_to_fit();
	}
}


#undef TRACE
#define TRACE fbx_printf("%s:%d\n", __FILE__,__LINE__);
void	file_trace_line(ifstream& src) {

	auto pos=src.tellg();
	int	i;
	cout<<"FILE["<<pos<<"]:";
	for (i=0; i<30; i++) { char c[2];c[0]=src.get();c[1]=0;cout<<c; } cout <<"\n";
	src.seekg(pos);
}
void	LoadFbxDeformer(FbxScene* scn, const char* modelName, const char* modelType, ifstream& src)
{
	char className[256],dstModelName[256], boneName[256];

	fbx_printf("%s %s\n", modelName, modelType);
	int	n=sscanf(modelName, "%s %s %s",className, dstModelName, boneName);
	if (n<3) {
		fbx_printf("incomplete deformer def?\n");
		return;
	}

	fbx_printf("deformer - (%s) (%s) (%s)\n", className, dstModelName, boneName);

	char tmp[256]; sprintf(tmp, "Model::%s", dstModelName);	// irritating FBX format
	fbx_printf("get model %s:-\n", dstModelName);
    FbxScene::Model*	mdl = scn->GetModel(dstModelName);
	if (!mdl) mdl=scn->GetModel(tmp);
	if (!mdl) {	fbx_printf("error could not link deformer to model %s\n",dstModelName); return;}
	vector<int>	indices;
	vector<float>	weights;
	char qualifiedBoneName[256]; sprintf(qualifiedBoneName, "Model::%s", boneName);
	int	boneIndex = scn->GetIndexOfModel(qualifiedBoneName);
	if (boneIndex<0) boneIndex=scn->GetIndexOfModel(boneName);
    FbxScene::Model*	boneMdl = scn->GetModel(qualifiedBoneName);
	if (boneIndex<0) {
		return;
	}
	boneMdl->isDeformer=true;

    for (FbxSubBlocks hdr(src); hdr.get();)
	{
		if (hdr=="Indexes:")  {
			LoadFbxNumericArray<int>(&indices, src);
		}
		else if (hdr=="Weights:") {
			LoadFbxNumericArray<float>(&weights, src);
		}
	}
	if (indices.size()>0)
	{
		auto mesh=scn->GetMeshOfModel(mdl);
		if (!mesh->weightMap.size())  mesh->weightMap.resize(mesh->vertices.size());
		for (size_t i=0; i<indices.size(); i++)
		{
			mesh->weightMap[indices[i]].add(boneIndex, weights[i]);
		}
    }
}

void
LoadFbxObjects(FbxScene* scn, ifstream& src)
{
	fbx_printf("load fbx objects Begin\n");

    for (FbxSubBlocks hdr(src); hdr.get(); )
	{
		fbx_printf("%s\n",hdr.c_str());
		if (hdr=="Model:")
		{
			FbxString<512> modelName, modelType;
            ReadQuotedString(modelName, src);
			SkipComma(src);
            ReadQuotedString(modelType, src);
			fbx_printf("Load model %s %s {\n",modelName.c_str(),modelType.c_str());
            auto	mdl = new FbxScene::Model;
			scn->allModels.push_back(mdl);
			LoadFbxModel(scn, mdl, modelName, modelType, src);
			fbx_printf("} load model %s %s done\n",modelName.c_str(),modelType.c_str() );
		}
		 else if (hdr=="Deformer:")
		{
			FbxString<256> modelName, modelType;
            ReadQuotedString(modelName,src); SkipComma(src);
            ReadQuotedString(modelType,src);
			LoadFbxDeformer(scn,modelName, modelType, src);
		}

	}
	fbx_printf("loadfbxobjects End\n");

}

void
LoadFbx(FbxScene*   scn, ifstream& src)
{
    for (FbxHeader hdr; hdr.LoadNext(src);)
	{
		if (hdr=="Objects:")
			LoadFbxObjects(scn, src);
		else if (hdr=="Connections:")
			LoadFbxConnections(scn, src);
        else if (hdr=="Takes:")
			LoadFbxTakes(scn,src);
	}
	scn->Finalize();
}


