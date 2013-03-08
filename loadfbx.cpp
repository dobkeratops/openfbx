#include "loadfbx.h"

using namespace std;


void FatalError(const char* ) {
}


inline void	write(IWriter* dst, const FbxScene* scene) {scene->Write(dst);};

class   FbxScene::Loader : public FbxUtil
{
    FbxScene* scene;
    FbxStream& file;
public:
    Loader(FbxScene* s, FbxStream& f): scene(s),file(f){
        for (Header hdr; hdr.LoadNext(file);)
        {
            if (hdr=="Objects:")
                this->LoadObjects();
            else if (hdr=="Connections:")
                this->LoadConnections();
            else if (hdr=="Takes:")
                this->LoadTakes();
            else BlockUnused(hdr);
        }
        scene->PostLoadingSetup();
    };

    class	Header : public String<256>
    {
    public:
        Header() {}
        //bool	LoadNext(FbxStream& src);
        Header&operator=(const char*src) { strncpy(&this->at(0),src,256); return *this;}

        bool    LoadNext(FbxStream& src)
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
                if (IsSymbolStart(c)) {
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
                    } while (IsSymbolCont(c));
                }
                else src.get();
            }
            return	false;
        };



    };

    class   SubBlocks {
    public:
        FbxStream*   pf;
        SubBlocks(FbxStream& file) {
            pf=&file;
            EnterBlock(*pf);
        }
        Header   hdr;
        const char* c_str() const { return hdr.c_str();}
        bool operator==(const char* txt) {return hdr==txt;}
        bool Get(){
            if (pf->eof()) return false;
            return hdr.LoadNext(*pf);
        }
        ~SubBlocks(){
            ExitBlock(*pf);
        }
    };

    static void BlockUnused(const Header& sb) {
        printf("unused block %s\n",sb.c_str());
    }
    static void BlockUnused(const SubBlocks& sb) {
        BlockUnused(sb.hdr);
    }



    void
    LoadAnimCurves(Take* dst, const char* mdlName, const char* channelName)
    {

        int	keyCount=-1;
        for (SubBlocks hdr(file);hdr.Get();)
        {
            // channel accumulates sub-channel name
            if (hdr=="Channel:") {
                String<256> subn;
                ReadString(subn,file);
                std::string chn;
                chn=channelName;
                if (strlen(channelName)>0)
                    chn+="_";
                chn+= subn;
                fbx_printf("sub channel %s for model %s{\n", channelName, mdlName);
                LoadAnimCurves(dst, mdlName, chn.c_str());
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

                if (keyCount>=0 && IsNumber(file))
                {
                    while (IsNumber(file))
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



    void    LoadConnections()
    {
        for (SubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Connect:")
            {

                String<128> type,from,to;
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
                BlockUnused(hdr);
        };
    }


    void
    LoadTakes()
    {
        for (SubBlocks hdr(file); hdr.Get();)
        {
            if (!(hdr=="Take:"))  {
                BlockUnused(hdr); continue;
            }
            auto* take = new Take;
            scene->takes.push_back(take);
            ReadString(take->name,file);
            {
                for (SubBlocks hdr(file); hdr.Get();) {
                    if (hdr=="Model:") {
                        String<256>	mdlName;
                        ReadString(mdlName,file);
                        LoadAnimCurves(take, mdlName.c_str(), "");
                    }
                }
            }
            take->Setup();
        }
    }



    void	LoadModelProperties(Model* mdl)
    {
        for (auto pm=Model::s_Properties; pm->name; pm++)
            fbx_printf("%s", pm->name);
        for (SubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Property:")
            {
                String<256> name, type, unknown;
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
            }else BlockUnused(hdr);
        }
        mdl->localRotate*=(2.f*M_PI/360.f);

        mdl->CalcLocalMatrixFromSRT();
        fbx_printf(mdl->localMatrix);
    }

    void
    LoadModel(Model* mdl, const char modelName[], const char modelType[])
    {
        vector<int> facePerTri;
        mdl->name = modelName;
        int numPolyVertex=0;
        Mesh*	mesh=nullptr;
        for (SubBlocks hdr(file); hdr.Get();)
        {
            fbx_dumpline(file);
            if (hdr=="Properties60:") {
                LoadModelProperties(mdl);
            } else
            if (hdr=="Vertices:")
            {
                if (!mesh) mesh=scene->CreateMeshForModel(mdl);
                LoadNumericArray(mesh->Vertices,file);
            } else
            if (hdr=="PolygonVertexIndex:")
            {
                LoadNumericArray(mesh->PolygonVertexIndex,file);
            }
            else if (hdr=="LayerElementNormal:")
            {
                if (mesh->vertexNormals.size()>0) {
                    printf("WARNING current code assumes 1 normal 'layer'");
                }
                auto layerId = Read<int>(file);

                for (SubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="Normals:") {
                        LoadNumericArray(mesh->vertexNormals,file);
                    }else BlockUnused(hdr);
                }
            }

            else if (hdr=="LayerElementUV:")
            {
                auto layerId = Read<int>(file);
                auto  layer = fbxAppend(mesh->LayerElementUVs);
                for (SubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="UV:") {
                        LoadNumericArray(layer->UV,file);
                    } else if (hdr=="UVIndex:") {
                        LoadNumericArray(layer->UVIndex,file);
                    }else BlockUnused(hdr);
                }
            }
            else if (hdr=="LayerElementTexture:")
            {
                auto layer = Read<int>(file);
                auto layertex=fbxAppend(mesh->LayerElementTextures);
                for (SubBlocks hdr(file);hdr.Get();)
                {
                    if (hdr=="TextureID:") {
                        LoadNumericArray(layertex->TextureID,file);
                    }else BlockUnused(hdr);
                }
            }
            else BlockUnused(hdr);

        }
        if (mesh) mesh->PostLoadingSetup();
    }

    void	LoadDeformer(const char* modelName, const char* modelType)
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

        for (SubBlocks hdr(file); hdr.Get();)
        {
            if (hdr=="Indexes:")  {
                LoadNumericArray<int>(indices, file);
            }
            else if (hdr=="Weights:") {
                LoadNumericArray<float>(weights, file);
            }
            else BlockUnused(hdr);
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
    LoadTexture()
    {
        String<256> texName,texType;
        ReadString(texName,file);
        ReadString(texType,file);

        Texture tx;
        for (SubBlocks hdr(file); hdr.Get(); ){
            if (hdr=="FileName:") {
                ReadString(tx.filename,file);
            }
        }
        scene->textures.push_back(tx);
    }

    void
    LoadObjects()
    {
        for (SubBlocks hdr(file); hdr.Get(); )
        {
            if (hdr=="Model:")
            {
                String<512> modelName, modelType;
                ReadString(modelName, file);
                ReadString(modelType, file);
                fbx_printf("Load model %s %s {\n",modelName.c_str(),modelType.c_str());
                auto	mdl = new Model;
                scene->allModels.push_back(mdl);
                LoadModel(mdl, modelName, modelType);
                fbx_printf("} load model %s %s done\n",modelName.c_str(),modelType.c_str() );
            }
            else if (hdr=="Deformer:")
            {
                String<256> modelName, modelType;
                ReadString(modelName,file);
                ReadString(modelType,file);
                LoadDeformer(modelName, modelType);
            } else if (hdr=="Texture:") {
                LoadTexture();

            } else BlockUnused(hdr);
        }
    }
};

void
LoadFbx(FbxScene* pScene, FbxStream& file)
{
    FbxScene::Loader loader(pScene,file);
}


