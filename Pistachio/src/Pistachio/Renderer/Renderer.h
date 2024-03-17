#pragma once
#include "RendererBase.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "RenderTexture.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio\Asset\AssetManager.h"
#include "Pistachio/Renderer/EditorCamera.h"
#include "Pistachio\Allocators\AtlasAllocator.h"
#include "ShadowMap.h"
#include "Pistachio\Allocators\FreeList.h"
#include "BufferHandles.h"
#include "Material.h"
namespace Pistachio {
	/*
	* The New Renderer would probably have a huge Vertex Buffer or Vertex Buffer Array, same for index buffer
	* then on creation, we suballocate into it, so that we only call Bind() once or NUM_VERTEX_BUFFERS per frame (probably at the beginning),
	* and this would be so even for multiple render passes (shadow maps), we could probably grow a singular buffer if we run out of space
	* Having this system would also help if i ever consider using techniques like Multi-Draw Indirect later on to send virtually all
	* geometry for each pass in about 1-2 calls, especially since the pipeline state would most likely not change per pass
	* (except for transparency and similar stuff).
	* The standard progression for a frame would be:
	* Z-Prepass(Opaque) -> Shadow Map Pass(es) -> Clustered Forward Shading(Opaque) -> Sorting and Shading(Transparent)
	* Post-processing (Compute??) -> 2D and UIx
	* we would probably use entt to sort all meshes by Material first, then by transparency, although im unsure if they'll retain ordering
	* 
	*/
	
	struct FrameResource
	{
		ConstantBuffer transformBuffer;
		ConstantBuffer passCB; //updates every frame
		ConstantBuffer materialCB;
		StructuredBuffer LightSB;
		RHI::DynamicDescriptor* transformBufferDesc;
	};
	struct PISTACHIO_API Light {
		DirectX::XMFLOAT3 position;// for directional lights this is direction
		LightType type;
		DirectX::XMFLOAT3 color;
		float intensity;
		DirectX::XMFLOAT4 exData;
		DirectX::XMFLOAT4 rotation;
		
	};
	struct PISTACHIO_API ShadowCastingLight
	{
		Light light;
		DirectX::XMFLOAT4X4 projection[4]; // used for frustum culling
		Region shadowMap;
		
		ShadowCastingLight(DirectX::XMMATRIX* _projection, Region _region, Light _light, int numMatrices)
		{
			for (int i = 0; i < numMatrices; i++)
				DirectX::XMStoreFloat4x4(&projection[i] ,_projection[i]);
			shadowMap = _region;
			light = _light;
		}
	};
	using RegularLight = Light;
	struct PISTACHIO_API PassConstants
	{
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 InvView;
		DirectX::XMFLOAT4X4 Proj;
		DirectX::XMFLOAT4X4 InvProj;
		DirectX::XMFLOAT4X4 ViewProj;
		DirectX::XMFLOAT4X4 InvViewProj;
		DirectX::XMFLOAT4 EyePosW;
		//float ShadowMapSize;
		DirectX::XMFLOAT2 RenderTargetSize;
		DirectX::XMFLOAT2 InvRenderTargetSize;
		float NearZ;
		float FarZ;
		float TotalTime;
		float DeltaTime;
		DirectX::XMMATRIX lightSpaceMatrix[16];
		int numRegularlights;
		int numShadowlights;
		DirectX::XMFLOAT2 _pad;
	};
	struct PISTACHIO_API TransformData
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX normal;
	};
	class PISTACHIO_API Renderer {
	public:
		static void Shutdown();
		static void ChangeSkybox(const char* filename);
		static void BeginScene(PerspectiveCamera* cam);
		static void BeginScene(RuntimeCamera* cam, const DirectX::XMMATRIX& transform);
		static void BeginScene(EditorCamera& cam);
		static void Init(const char* skybox);
		static void EndScene();
		static void Submit(Mesh* mesh, Shader* shader,  Material* mat, int ID);
		static void AddShadowCastingLight(const ShadowCastingLight& light);
		static void AddLight(const RegularLight& light);
		inline static ShaderLibrary& GetShaderLibrary() { return shaderlib; }
		static Material DefaultMaterial;
		static const RendererVBHandle AllocateVertexBuffer(uint32_t size, const void* initialData=nullptr);
		static void FreeVertexBuffer(const RendererVBHandle handle);
		static const RendererIBHandle AllocateIndexBuffer(uint32_t size,  const void* initialData=nullptr);
		static void FreeIndexBuffer(const RendererIBHandle handle);
		static const RendererCBHandle AllocateConstantBuffer(uint32_t size);
		static void PartialCBUpdate(RendererCBHandle, void* data,uint32_t offset, uint32_t size);
		static void FullCBUpdate(RendererCBHandle handle,void* data);
		//todo: remove these
		static const uint32_t GetIBOffset(const RendererIBHandle handle);
		static const uint32_t GetVBOffset(const RendererVBHandle handle);
		static const uint32_t GetCBOffset(const RendererCBHandle handle);
		static void  Submit(RHI::GraphicsCommandList* lsit, const RendererVBHandle vb, const RendererIBHandle ib, uint32_t vertexStride);
		static const Texture2D& GetWhiteTexture();
		static RenderCubeMap& GetSkybox();
		static SamplerHandle GetDefaultSampler();
		static const RHI::DynamicDescriptor* GetCBDesc();
		static RHI::Buffer* GetVertexBuffer();
		static RHI::Buffer* GetIndexBuffer();
		static RHI::Buffer* GetConstantBuffer();
		static void OnEvent(Event& e) {
			if (e.GetEventType() == EventType::WindowResize)
				OnWindowResize((WindowResizeEvent&)e);
		}
		static void OnWindowResize(WindowResizeEvent& e)
		{
			
		}
		
	private:
		static uint32_t AssignHandle(
			std::vector<uint32_t>& offsetsVector,
			std::vector<uint32_t>& freeHandlesVector,
			std::uint32_t offset);
		static void CreateConstantBuffers();
		static void UpdatePassConstants();
		static void GrowMeshVertexBuffer(uint32_t minExtraSize);
		static void GrowMeshIndexBuffer(uint32_t minExtraSize);
		static void GrowConstantBuffer(uint32_t minExtraSize);


		static void DefragmentMeshVertexBuffer();
		static void DefragmentMeshIndexBuffer();
		static void DefragmentConstantBuffer();
		inline static RendererVBHandle AllocateBuffer(
			FreeList& flist,
			uint32_t& free_space,
			uint32_t& fast_space,
			uint32_t& capacity,
			decltype(&Renderer::GrowMeshVertexBuffer) grow_fn,
			decltype(&Renderer::DefragmentMeshVertexBuffer) defrag_fn,
			std::vector<uint32_t>& offsetsVector,
			std::vector<uint32_t>& freeHandlesVector,
			RHI::Buffer** buffer,
			uint32_t size, 
			const void* initialData=nullptr);
	private:
		static void (*CBInvalidated)();
		//New Renderer
		static RHI::Buffer* meshVertices; // all meshes in the scene?
		static RHI::Buffer*  meshIndices;
		static uint32_t     vbFreeFastSpace;//free space for an immerdiate allocation
		static uint32_t     vbFreeSpace;   //total free space to consider reordering
		static uint32_t     vbCapacity;
		static FreeList     vbFreeList;
		/*
		 * handles map buffer handles to thier actual offsets, in case defragmentation moves them around
		 * each handle is just an offset into this vector
		 */
		static std::vector<uint32_t> vbHandleOffsets;
		// in the case we free a vertex buffer, we dont want to have to resize the offset vector even when there are unused spaces
		static std::vector<uint32_t> vbUnusedHandles;
		static uint32_t     ibFreeFastSpace;
		static uint32_t     ibFreeSpace;
		static uint32_t     ibCapacity;
		static FreeList     ibFreeList;
		static std::vector<uint32_t> ibHandleOffsets;
		static std::vector<uint32_t> ibUnusedHandles;
		/*
		 * Constant Buffers are a little bit tricky
		 * Handling updates to all buffers is the job of the caller
		 * but handling allocations and defragments is the job of the renderer
		 * An option would be to keep a uint for every frame that needs an op
		 * eg uint32_t numDirtyAllocs; OnUpdate(){if(numDirtyAllocs){}}
		 * but then we'd have to store op's order and details.
		 * We'd probably wait for the gpu to get to the current frame, before doing any op's except updating
		 * since updating is handled by the scene, it'll have the data needed to recreate the update event
		 * 
		 * Im leaving the above comment, but i might just have found a better way.
		 * Allocate wouldn't update just give you a handle, and you update for all three buffers
		 * Deallocate would just update the free list
		 * Grow and Defrag would however block
		 */
		
		static uint32_t     cbCapacity;
		static uint32_t     cbFreeSpace;
		static uint32_t     cbFreeFastSpace;
		static FreeList     cbFreeList;
		static uint32_t     numDirtyCBFrames;
		static std::vector<uint32_t> cbHandleOffsets;
		static std::vector<uint32_t> cbUnusedHandles;
		static FrameResource resources[3];
		static RenderCubeMap skybox;
		static RenderCubeMap irradianceSkybox;
		static RenderCubeMap prefilterSkybox;
		static RenderTexture BrdfTex;
		static Shader* eqShader;//equirectangular to cube map
		static Shader* irradianceShader;
		static Shader* prefilterShader;
		static SetInfo eqShaderVS[6];
		static SetInfo eqShaderPS;
		static SetInfo irradianceShaderPS;
		static SetInfo prefilterShaderVS[5];

		static SamplerHandle defaultSampler;

		static DirectX::XMMATRIX viewproj;
		static DirectX::XMVECTOR m_campos;
		static ShaderLibrary shaderlib;
		static ConstantBuffer MaterialCB;						//temporary todo: rework renderer
		static ConstantBuffer PassCB;							//temporary todo: rework renderer
		static StructuredBuffer LightSB;						//temporary todo: rework renderer
		static std::vector<ConstantBuffer> TransformationBuffer;//temporary todo: rework renderer
		static struct CamerData { DirectX::XMMATRIX viewProjection; DirectX::XMMATRIX view;  DirectX::XMFLOAT4 viewPos; }CameraData;
		static Texture2D whiteTexture;
		static PassConstants passConstants;
		static std::vector<RegularLight> RegularLightData;
		static std::vector<ShadowCastingLight> ShadowLightData;
		static std::vector<std::uint8_t> LightSBCPU;
		static Material* currentMat;
		static Shader* currentShader;
		static DepthTexture shadowMapAtlas;
		friend class Scene;
		friend class Material;
		static Shader* brdfShader;
	};
}
