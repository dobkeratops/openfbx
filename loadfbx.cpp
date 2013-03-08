#include "loadfbx.h"

using namespace std;
typedef ifstream FbxStream;

class	FbxHeader : public FbxString<256>
{
public:
    FbxHeader() {}
    bool	LoadNext(ifstream& file);
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


#undef TRACE
#define TRACE fbx_printf("%s:%d\n", __FILE__,__LINE__);
void	file_trace_line(FbxStream& src) {

    auto pos=src.tellg();
    int	i;
    cout<<"FILE["<<pos<<"]:";
    for (i=0; i<30; i++) { char c[2];c[0]=src.get();c[1]=0;cout<<c; } cout <<"\n";
    src.seekg(pos);
}

inline void	write(IWriter* dst, const FbxScene* scene) {scene->Write(dst);};
//inline bool StrEq(const char*a ,const char* b) { return !strcmp(a,b); }
//inline bool	SkipComma(ifstream& src)
//{
//	char c; src>>c; if (c==',') return true; else src.unget(); return false;
//}



class   FbxScene::Loader
{
    FbxScene* scene;
    FbxStream& file;
public:
    Loader(FbxScene* s, FbxStream& f): scene(s),file(f){
        for (FbxHeader hdr; hdr.LoadNext(file);)
        {
            if (hdr=="Objects:")
                this->LoadFbxObjects();
            else if (hdr=="Connections:")
                this->LoadFbxConnections();
            else if (hdr=="Takes:")
                this->LoadFbxTakes();
            else fbxBlockUnused(hdr);
        }
        scene->PostLoadingSetup();
    };
    void
    LoadFbxAnimCurves(Take* dst, const char* mdlName, const char* channelName)
    {

        int	keyCount=-1;
        for (FbxSubBlocks hdr(file);hdr.Get();)
        {
            // channel accumulates sub-channel name
            if (hdr=="Channel:") {
                FbxString<256> subn;
                ReadString(subn,file);
                std::string chn;
                chn=channelName;
                if (strlen(channelName)>0)
                    chn+="_";
                chn+= subn;
                fbx_printf("sub channel %s for model %s{\n", channelName, mdlName);
                LoadFbxAnimCurves(dst, mdlName, chn.c_str());
                fbx_printf("}%s %s \n", channelName, mdlName);
            } else
            if (hdr=="KeyCount:")
            {
                ASSERT(keyCount==-1);
                file>>keyCount;
            }
            else if (hdr=="Key:")
            {
                FCurve newCurve;
                ASSERT(keyCount>=0)
                int	i=0;
                newCurve.boneName=mdlName;
                newCurve.channelName = channelName;
                newCurve.modelIndex = scene->GetIndexOfModel(mdlName);
                newCurve.channelIndex = GetChannelIndex(channelName);

                if (keyCount>=0 && fbxIsNumber(file))
                {
                    while (fbxIsNumber(file))
                    {
                        auto t=Read<float>(file);
                        auto f=Read<float>(file);
                        auto l=Read<char>(file);

                        fbx_printf("channel data:\t%lf\t%lf\t%c\n", t,f,l);
                        newCurve.points.push_back(FCurve::tvalue(t,f));
                    };
                    newCurve.points.shrink_to_fit();
                }

                dst->curves.push_back(newCurve); // todo emplace_back
            }
        }
    }



    void    LoadFbxConnections()
    {
        for (FbxSubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Connect:")
            {

                FbxString<128> type,from,to;
                ReadString(type,file);
                ReadString(from,file);
                ReadString(to,file);
                auto fromMdl = scene->GetModel(from);
                if (!(type=="OO"))
                    continue;
                vector<Model*>* toMdlList;
                if (to=="Model::Scene")
                    toMdlList = &scene->rootModels;
                else {
                    auto toMdl = scene->GetModel(to);
                    if (!toMdl) continue;   // cant' link, its not loaded.
                    toMdlList =&toMdl->childModels;
                    if (fromMdl)
                        fromMdl->parent = toMdl;
                }
                if (fromMdl)
                {
                    toMdlList->push_back(fromMdl);
                    scene->hrcLinks.push_back(HrcLink(scene->GetIndexOfModel(from),scene->GetIndexOfModel(to)));
                }
                // TODO: Some of these connections associate the deformers
                // we might have made assumptions on deformer connection from the names

                fbx_printf("Connected(%s)\t%s\t->\t%s\n", type.c_str(), from.c_str(), to.c_str());
            } else
                fbxBlockUnused(hdr);
        };
    }


    void
    LoadFbxTakes()
    {
        for (FbxSubBlocks hdr(file); hdr.Get();)
        {
            if (!(hdr=="Take:"))  {
                fbxBlockUnused(hdr); continue;
            }
            auto* take = new Take;
            scene->takes.push_back(take);
            ReadString(take->name,file);
            {
                for (FbxSubBlocks hdr(file); hdr.Get();) {
                    if (hdr=="Model:") {
                        FbxString<256>	mdlName;
                        ReadString(mdlName,file);
                        LoadFbxAnimCurves(take, mdlName.c_str(), "");
                    }
                }
            }
            take->Setup();
        }
    }



    void	LoadFbxModelProperties(Model* mdl)
    {
        for (auto pm=Model::s_Properties; pm->name; pm++)
            fbx_printf("%s", pm->name);
        for (FbxSubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Property:")
            {
                FbxString<256> name, type, unknown;
                ReadString(name,file);
                ReadString(type, file);
                ReadString(unknown,file);
                if (name=="Lcl Translation") {
                    mdl->localTranslate = Read<Vector3>(file);

                } else
                if (name=="Lcl Rotation") {
                    mdl->localRotate = Read<Vector3>(file);
                } else
                if (name=="Lcl Scaling") {
                    mdl->localScale = Read<Vector3>(file);
                }

                // else - parameter table for defaults.
            }else fbxBlockUnused(hdr);
        }
        mdl->localRotate*=(2.f*M_PI/360.f);

        mdl->CalcLocalMatrixFromSRT();
        fbx_printf(mdl->localMatrix);
    }

    void
    LoadFbxModel(Model* mdl, const char modelName[], const char modelType[])
    {
        vector<int> facePerTri;
        mdl->name = modelName;
        int numPolyVertex=0;
        Mesh*	mesh=nullptr;
        for (FbxSubBlocks hdr(file); hdr.Get();)
        {
            fbx_dumpline(file);
            if (hdr=="Properties60:") {
                LoadFbxModelProperties(mdl);
            } else
            if (hdr=="Vertices:")
            {
                if (!mesh) mesh=scene->CreateMeshForModel(mdl);
                FbxLoadNumericArray(mesh->Vertices,file);
            } else
            if (hdr=="PolygonVertexIndex:")
            {
                FbxLoadNumericArray(mesh->PolygonVertexIndex,file);
            }
            else if (hdr=="LayerElementNormal:")
            {
                if (mesh->vertexNormals.size()>0) {
                    printf("WARNING current code assumes 1 normal 'layer'");
                }
                auto layerId = Read<int>(file);

                for (FbxSubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="Normals:") {
                        FbxLoadNumericArray(mesh->vertexNormals,file);
                    }else fbxBlockUnused(hdr);
                }
            }

            else if (hdr=="LayerElementUV:")
            {
                auto layerId = Read<int>(file);
                auto  layer = fbxAppend(mesh->LayerElementUVs);
                for (FbxSubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="UV:") {
                        FbxLoadNumericArray(layer->UV,file);
                    } else if (hdr=="UVIndex:") {
                        FbxLoadNumericArray(layer->UVIndex,file);
                    }else fbxBlockUnused(hdr);
                }
            }
            else if (hdr=="LayerElementTexture:")
            {
                auto layer = Read<int>(file);
                auto layertex=fbxAppend(mesh->LayerElementTextures);
                for (FbxSubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="TextureID:") {
                        FbxLoadNumericArray(layertex->TextureID,file);
                    }else fbxBlockUnused(hdr);
                }
            }
            else fbxBlockUnused(hdr);

        }
        if (mesh) mesh->PostLoadingSetup();
    }

    void	LoadFbxDeformer(const char* modelName, const char* modelType)
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
        auto	mdl = scene->GetModel(dstModelName);
        if (!mdl) mdl=scene->GetModel(tmp);
        if (!mdl) {	fbx_printf("error could not link deformer to model %s\n",dstModelName); return;}
        vector<int>	indices;
        vector<float>	weights;
        char qualifiedBoneName[256]; sprintf(qualifiedBoneName, "Model::%s", boneName);
        int	boneIndex = scene->GetIndexOfModel(qualifiedBoneName);
        if (boneIndex<0) boneIndex=scene->GetIndexOfModel(boneName);
        auto	boneMdl = scene->GetModel(qualifiedBoneName);
        if (boneIndex<0) {
            return;
        }
        boneMdl->isDeformer=true;

        for (FbxSubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Indexes:")  {
                FbxLoadNumericArray<int>(indices, file);
            }
            else if (hdr=="Weights:") {
                FbxLoadNumericArray<float>(weights, file);
            }
            else fbxBlockUnused(hdr);
        }
        if (indices.size()>0)
        {
            auto mesh=scene->GetMeshOfModel(mdl);
            auto &wmap=mesh->vertexWeightMap;
            if (!wmap.size())  wmap.resize(mesh->Vertices.size());
            for (size_t i=0; i<indices.size(); i++)
            {
                wmap[indices[i]].add(boneIndex, weights[i]);
            }
        }
    }

    void
    LoadFbxTexture()
    {
        FbxString<256> texName,texType;
        ReadString(texName,file);
        ReadString(texType,file);

        Texture tx;
        for (FbxSubBlocks hdr(file); hdr.Get(); ){
            if (hdr=="FileName:") {
                ReadString(tx.filename,file);
            }
        }
        scene->textures.push_back(tx);
    }

    void
    LoadFbxObjects()
    {
        for (FbxSubBlocks hdr(file); hdr.Get(); )
        {
            if (hdr=="Model:")
            {
                FbxString<512> modelName, modelType;
                ReadString(modelName, file);
                ReadString(modelType, file);
                fbx_printf("Load model %s %s {\n",modelName.c_str(),modelType.c_str());
                auto	mdl = new Model;
                scene->allModels.push_back(mdl);
                LoadFbxModel(mdl, modelName, modelType);
                fbx_printf("} load model %s %s done\n",modelName.c_str(),modelType.c_str() );
            }
            else if (hdr=="Deformer:")
            {
                FbxString<256> modelName, modelType;
                ReadString(modelName,file);
                ReadString(modelType,file);
                LoadFbxDeformer(modelName, modelType);
            } else if (hdr=="Texture:") {
                LoadFbxTexture();

            } else fbxBlockUnused(hdr);
        }
    }
};

void
LoadFbx(FbxScene* pScene, FbxStream& file)
{
    FbxScene::Loader loader(pScene,file);
}


