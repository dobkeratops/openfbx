#include "loadfbx.h"

using namespace std;
typedef ifstream FbxStream;

class	FbxHeader : public FbxString<256>
{
public:
    FbxHeader() {}
    bool	LoadNext(ifstream& src);
    FbxHeader&operator=(const char*src) { strncpy(&this->at(0),src,256); return *this;}
};

class   FbxSubBlocks {
public:
    FbxStream*   pf;
    FbxSubBlocks(FbxStream& file) {
        pf=&file;
        EnterBlock(*pf);
    }
    FbxHeader   hdr;
    const char* c_str() const { return hdr.c_str();}
    bool operator==(const char* txt) {return hdr==txt;}
    bool Get(){
        if (pf->eof()) return false;
        return hdr.LoadNext(*pf);
    }
    ~FbxSubBlocks(){
        ExitBlock(*pf);
    }
};

template<typename T>
void	FbxLoadNumericArray(vector<T>&	dst, FbxStream& src)
{   while(fbxIsNumber(src)) {
        dst.push_back(Read<T>(src));
    }
    dst.shrink_to_fit();
}


void FatalError(const char* ) {
}



void
LoadFbxAnimCurves(FbxScene* scn, FbxStream& src, FbxScene::Take* dst, const char* mdlName, const char* channelName)
{

	int	keyCount=-1;
    for (FbxSubBlocks hdr(src);hdr.Get();)
	{
		// channel accumulates sub-channel name
		if (hdr=="Channel:") {
			FbxString<245> subn;
            ReadString(subn,src);
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
            FbxScene::FCurve newCurve;
			ASSERT(keyCount>=0)
			int	i=0;
			newCurve.boneName=mdlName;
			newCurve.channelName = channelName;
			newCurve.modelIndex = scn->GetIndexOfModel(mdlName);
            newCurve.channelIndex = FbxScene::GetChannelIndex(channelName);

            if (keyCount>=0 && fbxIsNumber(src))
			{
                while (fbxIsNumber(src))
				{
                    auto t=Read<float>(src);
                    auto f=Read<float>(src);
                    auto l=Read<char>(src);

                    fbx_printf("channel data:\t%lf\t%lf\t%c\n", t,f,l);
                    newCurve.points.push_back(FbxScene::FCurve::tvalue(t,f));
                };
				newCurve.points.shrink_to_fit();
			}

			dst->curves.push_back(newCurve); // todo emplace_back

		}
	}
}


bool
FbxHeader::LoadNext(FbxStream& src)
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
                    fbx_printf("Read Subblock %s\n",hdr->c_str());
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
void fbxBlockUnused(const FbxHeader& sb) {
    printf("unused block %s\n",sb.c_str());
}
void fbxBlockUnused(const FbxSubBlocks& sb) {
    fbxBlockUnused(sb.hdr);
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

FbxScene::Vector3	ReadVector3(FbxStream& src) {
    auto x=Read<float>(src), y=Read<float>(src), z=Read<float>(src);
    return {{x,y,z}};
}

void    LoadFbxConnections(FbxScene* scn, FbxStream& file)
{
    fbx_printf("Connections{");

    for (FbxSubBlocks hdr(file); hdr.Get();)
    {
        if (hdr=="Connect:")
        {

            FbxString<128> type,from,to;
            ReadString(type,file);
            ReadString(from,file);
            ReadString(to,file);
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
        } else
            fbxBlockUnused(hdr);
    };
    fbx_printf("Connections}");
}


void
LoadFbxTakes(FbxScene* scn, FbxStream& src)
{
    for (FbxSubBlocks hdr(src); hdr.Get();)
	{
        if (!(hdr=="Take:"))  {
            fbxBlockUnused(hdr); continue;
        }
        auto* take = new FbxScene::Take;
		scn->takes.push_back(take);
        ReadString(take->name,src);
		{
            for (FbxSubBlocks hdr(src); hdr.Get();) {
				if (hdr=="Model:") {
					FbxString<256>	mdlName;
                    ReadString(mdlName,src);
					LoadFbxAnimCurves(scn, src, take, mdlName.c_str(), "");
				}
            }
		}
        take->Setup();
    }
	fbx_printf("debug, test parsing of LoadFbxTakes");
}



void	LoadFbxModelProperties(FbxScene* scn, FbxScene::Model* mdl, FbxStream & src)
{
    for (auto pm=FbxScene::Model::s_Properties; pm->name; pm++)
		fbx_printf("%s", pm->name);
    for (FbxSubBlocks hdr(src); hdr.Get();)
	{
		if (hdr=="Property:")
		{
			FbxString<256> name, type, unknown;
            ReadString(name,src);
            ReadString(type, src);
            ReadString(unknown,src);
			if (name=="Lcl Translation") {
                mdl->localTranslate = ReadVector3(src);

			} else
			if (name=="Lcl Rotation") {
                mdl->localRotate = ReadVector3(src);
			} else
			if (name=="Lcl Scaling") {
                mdl->localScale = ReadVector3(src);
			}

			// else - parameter table for defaults.
        }else fbxBlockUnused(hdr);
	}
	mdl->localRotate*=(2.f*M_PI/360.f);

	mdl->CalcLocalMatrixFromSRT();
	fbx_printf(mdl->localMatrix);
}

void
LoadFbxModel(FbxScene* pscn, FbxScene::Model* mdl, const char modelName[], const char modelType[], FbxStream& src)
{
    vector<int> facePerTri;
    mdl->name = modelName;
    int numPolyVertex=0;
    FbxScene::Mesh*	mesh=nullptr;
    for (FbxSubBlocks hdr(src); hdr.Get();)
	{
		fbx_dumpline(src);
		if (hdr=="Properties60:") {
			LoadFbxModelProperties(pscn,mdl, src);
        } else
		if (hdr=="Vertices:")
		{
            if (!mesh) mesh=pscn->CreateMeshForModel(mdl);
            FbxLoadNumericArray(mesh->Vertices,src);
        } else
		if (hdr=="PolygonVertexIndex:")
		{
            FbxLoadNumericArray(mesh->PolygonVertexIndex,src);
        }
        else if (hdr=="LayerElementNormal:")
        {
            if (mesh->vertexNormals.size()>=0) {
                printf("WARNING current code assumes 1 normal 'layer'");
            }
            auto layerId = Read<int>(src);

            for (FbxSubBlocks hdr(src);hdr.Get();)
            {
                if (hdr=="Normals:") {
                    FbxLoadNumericArray(mesh->vertexNormals,src);
                }else fbxBlockUnused(hdr);
            }
        }

        else if (hdr=="LayerElementUV:")
        {
            auto layerId = Read<int>(src);
            auto  layer = fbxAppend(mesh->LayerElementUVs);
            for (FbxSubBlocks hdr(src);hdr.Get();)
            {
                if (hdr=="UV:") {
                    FbxLoadNumericArray(layer->UV,src);
                } else if (hdr=="UVIndex:") {
                    FbxLoadNumericArray(layer->UVIndex,src);
                }else fbxBlockUnused(hdr);
            }
        }
        else if (hdr=="LayerElementTexture:")
        {
            auto layer = Read<int>(src);
            auto layertex=fbxAppend(mesh->LayerElementTextures);
            for (FbxSubBlocks hdr(src);hdr.Get();)
            {
                if (hdr=="TextureID:") {
                    FbxLoadNumericArray(layertex->TextureID,src);
                }else fbxBlockUnused(hdr);
            }
        }
        else fbxBlockUnused(hdr);

	}
    if (mesh) mesh->PostLoadingSetup();
}



#undef TRACE
#define TRACE fbx_printf("%s:%d\n", __FILE__,__LINE__);
void	file_trace_line(FbxStream& src) {

	auto pos=src.tellg();
	int	i;
	cout<<"FILE["<<pos<<"]:";
	for (i=0; i<30; i++) { char c[2];c[0]=src.get();c[1]=0;cout<<c; } cout <<"\n";
	src.seekg(pos);
}
void	LoadFbxDeformer(FbxScene* scn, const char* modelName, const char* modelType, FbxStream& src)
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

    for (FbxSubBlocks hdr(src); hdr.Get();)
	{
		if (hdr=="Indexes:")  {
            FbxLoadNumericArray<int>(indices, src);
		}
		else if (hdr=="Weights:") {
            FbxLoadNumericArray<float>(weights, src);
		}
        else fbxBlockUnused(hdr);
	}
	if (indices.size()>0)
	{
		auto mesh=scn->GetMeshOfModel(mdl);
        auto &wmap=mesh->vertexWeightMap;
        if (!wmap.size())  wmap.resize(mesh->Vertices.size());
		for (size_t i=0; i<indices.size(); i++)
		{
            wmap[indices[i]].add(boneIndex, weights[i]);
		}
    }
}

void
LoadFbxTexture(FbxScene* scn, FbxStream& src)
{
    FbxString<256> texName,texType;
    ReadString(texName,src);
    ReadString(texType,src);

    FbxScene::Texture tx;
    for (FbxSubBlocks hdr(src); hdr.Get(); ){
        if (hdr=="FileName:") {
            ReadString(tx.filename,src);
        }
    }
    scn->textures.push_back(tx);
}

void
LoadFbxObjects(FbxScene* scn, FbxStream& src)
{
	fbx_printf("load fbx objects Begin\n");

    for (FbxSubBlocks hdr(src); hdr.Get(); )
	{
		if (hdr=="Model:")
		{
			FbxString<512> modelName, modelType;
            ReadString(modelName, src);
            ReadString(modelType, src);
			fbx_printf("Load model %s %s {\n",modelName.c_str(),modelType.c_str());
            auto	mdl = new FbxScene::Model;
			scn->allModels.push_back(mdl);
			LoadFbxModel(scn, mdl, modelName, modelType, src);
			fbx_printf("} load model %s %s done\n",modelName.c_str(),modelType.c_str() );
		}
        else if (hdr=="Deformer:")
		{
            FbxString<256> modelName, modelType;
            ReadString(modelName,src);
            ReadString(modelType,src);
			LoadFbxDeformer(scn,modelName, modelType, src);
        } else if (hdr=="Texture:") {
            LoadFbxTexture(scn,src);

        } else fbxBlockUnused(hdr);

	}
	fbx_printf("loadfbxobjects End\n");
}

void
LoadFbx(FbxScene*   scn, ifstream& file)
{
    FbxStream& src=file;
    for (FbxHeader hdr; hdr.LoadNext(src);)
	{
		if (hdr=="Objects:")
			LoadFbxObjects(scn, src);
		else if (hdr=="Connections:")
			LoadFbxConnections(scn, src);
        else if (hdr=="Takes:")
			LoadFbxTakes(scn,src);
        else fbxBlockUnused(hdr);
	}
    scn->PostLoadingSetup();
}


