#ifndef fbxscene_h
#define fbxscene_h

#include<memory>
#include "fbxutil.h"
#include "fbxmath.h"


/*
translator to structured ascii output eg JSON
*/



class	FbxScene : public FbxMath
{
public:
    class   Loader;
	static Channel_t	GetChannelIndex(const char* name);
	static const char* getChannelName(int index);
	
	struct	HrcLink {
		HrcLink(int a,int b) {child=a; parent=b;}
		int parent,child;
	};

    typedef Vector3	 Vertex;
    struct	SRT {
        Vector3	scale,rotate,translate;
    };

	enum RotationOrder_t {
		RO_XmYZ
	};
    struct	Triangle//: public std::array<int,3>
	{
        std::array<int,3> vertex;
		Triangle() {};
        Triangle(int i0,int i1, int i2) { auto &t=*this; t.vertex[0]=i0; t.vertex[1]=i1; t.vertex[2]=i2;}
        int getVertex(int i)const{return vertex[i%3];}
        int texId;
		void	Write(IWriter*) const;
	};
	struct	Quad : public std::array<int,4>
	{
		Quad() {};
		Quad(int i0,int i1, int i2,int i3) {auto &t=*this; t[0]=i0; t[1]=i1; t[2]=i2; t[3]=i3;}
		void	Write(IWriter*) const;
	};
	template<typename OBJECT>
	struct	PropertyDef { const char* name; int dim; float OBJECT::*offset; };

	struct	VertexBoneWeights 
	{
		enum {MAX=4};
		std::array<int,MAX>	boneIndex;
		std::array<float,MAX>	boneWeight;
		int	num;
		int	size() const { return num;}
		int capacity() const { return MAX;}
		void	add(int index, float weight);
		VertexBoneWeights() { num=0; int i; for(i=0;i<MAX;i++) boneWeight[i]=0.f;}
		void	Write(IWriter*) const;
	};

    enum {MaxUVLayers=2};
    enum {MaxBones=4};

    typedef int VertexIndex_t;
    class   RenderVertex{
    public:
        VertexIndex_t posIndex;
        Vector2 texcoord[MaxUVLayers];
        Vector3 normal;
        int     color;
        // weight map is indexed via 'position'
    };

    class	Mesh
	{
	public:
        class LayerElementUV {
        public:
            std::vector<Vector2> UV;
            std::vector<int>    UVIndex;
        };
        class LayerElementTexture {
        public:
            std::vector<int> TextureID;
        };
        class LayerElementNormal {
        public:
            std::vector<Vector3> Normals;
        };

        // raw indexed representation of faces & layers from FBX file
        std::vector<Vertex>         Vertices;
        std::vector<VertexIndex_t>            PolygonVertexIndex;
        VertexIndex_t   GetPolygonVertexIndex(int i) { auto vi=PolygonVertexIndex[i]; if (vi<0) vi=~vi; return vi;}
        bool            IsFaceEnd(int i)            { return PolygonVertexIndex[i]<0;}
//        LayerElementNormal normals;
        std::vector<Vector3>        vertexNormals;
        std::vector<LayerElementUV> LayerElementUVs;
        std::vector<LayerElementTexture>            LayerElementTextures;
        // todo -materials, smoothing, vertex-colours, various options in there we've missed

        // TODO: split these processed versions out.
        // add preprocess for rendering vertices.
        // optonal quadlist
        std::vector<VertexBoneWeights>  vertexWeightMap; // assembled from deformers..
        std::vector<Triangle>	triangles;
        std::vector<RenderVertex>   renderVertex;
        std::vector<Triangle>   renderTriangles;
        Mesh() {}
    private:
        void MakeRenderable();
    public:
		void	NormalizeWeightMap();
        void	PostLoadingSetup();

    };
    class Texture {
    public:
        String<256> filename;
    };

    class	Model
	{
	public:
        String<128>   name;			// todo: seperate array.
		Vector3	localTranslate,localRotate,localScale;
		float	GetChannel(Channel_t c) const;
		Matrix  localMatrix;
		RotationOrder_t	rotOrder;
		Vector4	weightMapColor;
        Model*			parent;
        std::vector<Model*>    childModels;
        static	PropertyDef<Model>	s_Properties[];
        bool	isDeformer;
        unique_ptr<Mesh>    mesh;

        Matrix  GetGlobalMatrix() const;
		Matrix GetLocalMatrix() const;
        //bool	HasMesh() const { return meshId>=0;}
        Model();
        ~Model() {}
		void	CalcLocalMatrixFromSRT();
        Matrix	GetLocalMatrixPermuteTest(int permute) const;
        Mesh* GetMesh(){ return mesh.get();}
	};
    typedef std::vector<std::array<float, NumChannels> > CycleEvalBuffer;

	std::string	name;
    std::vector<Model*> allModels;	// collection owns all models
    std::vector<Model*> rootModels;	// heirarchy roots
    //std::vector<FbxMesh>    meshes;
    std::vector<HrcLink>    hrcLinks;	// parent-child relations, "object-owner"?
    std::vector<Texture>    textures;
    Extents3 extents;

    Model*	CreateModel() { auto mdl=new Model(); allModels.push_back(mdl); return mdl;}
    Mesh*   CreateMeshForModel(Model* mdl);
    //const Mesh*	GetMeshOfModel(const Model* mdl) const { return mdl->meshId>=0?&this->meshes[mdl->meshId]:nullptr;}
    Mesh*	GetMeshOfModel(Model* mdl) { return mdl->mesh.get();}
    const Mesh*	GetMeshOfModel(const Model* mdl) const { return mdl->mesh.get();}
    Model*  GetModel(const char* mdlName);
	int  GetIndexOfModel(const char* mdlName);
    void	PostLoadingSetup();
	static const char*	GetRawModelName(const char* src);	// remove namespace qualifier Model::blah
    void    UpdateExtents(Extents3& ext, const Model* mdl, const Matrix& parent);

	struct	FCurve
    {
        struct	tvalue { float t, value;
            tvalue(float _t, float _v) {t=_t; value=_v;}
        };
        String<128> boneName, channelName;
		int	modelIndex; Channel_t channelIndex;
        std::vector<tvalue> points;
	};
	struct	StaticBonePose { 
		short	boneIndex; char component; char subComponent;
		float value;
	};
	class	Take 
	{
	public:
        String<256>	name;
		std::vector<StaticBonePose> staticBones;
		std::vector<FCurve>	curves;
        void    Setup();
        float   maxt;
	};
	std::vector<Take*>	takes;

    FbxScene() { }
    ~FbxScene () { for (auto& m :this->allModels) delete m;}


	void	InitCycleEvalBuffer(CycleEvalBuffer& dst) const;
	void	EvalTake(CycleEvalBuffer& dst, const Take* src, float t) const;	// todo - accumulation
	void	EvalFCurve(CycleEvalBuffer& dst, const FCurve* curve, float t) const;
    void	EvalMatrixArray(Matrix* dst, const CycleEvalBuffer* src) const;
	void	Write(IWriter* dst) const;
    static const char* s_ChannelNames[];
    static Vector4	s_WeightMapColors[4];
    typedef float Model::*ModelFloatPtr_t;
    float Radius() const;
    Vector3 Centre() const;
};
void    FbxDumpModel(const FbxScene* scn, const FbxScene::Model* mdl, const FbxMath::Matrix& p);
void	FbxDumpScene(const FbxScene* scn,IWriter* out) ;





// todo,move this somewhere

#endif /* fbxscene_h */
