#pragma once
#include "Barrier.h"
#include "FormatsAndTypes.h"
#include "Pistachio/Renderer/RenderGraph.h"
#include "Pistachio/Renderer/RendererContext.h"
#include "Ptr.h"
#include "RendererBase.h"
#include "Shader.h"
#include "RenderTexture.h"
#include "Texture.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Asset/AssetManager.h"
#include "Pistachio/Allocators/AtlasAllocator.h"
#include "Pistachio/Allocators/FreeList.h"
#include "BufferHandles.h"
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
	class Material;
	
	struct PISTACHIO_API Light {
		DirectX::XMFLOAT3 position;
		LightType type;
		DirectX::XMFLOAT3 color;
		float intensity;
		DirectX::XMFLOAT4 exData;
		DirectX::XMFLOAT4 rotation;
		
	};
	struct ComputeShaderMisc
	{
		uint32_t counter;
	};
	struct PISTACHIO_API ShadowCastingLight
	{
		Light light;
		DirectX::XMFLOAT4X4 projection[4]; // used for frustum culling
		Region shadowMap;
		ShadowCastingLight() = default;
		ShadowCastingLight(DirectX::XMMATRIX* _projection, Region _region, Light _light, int numMatrices)
		{
			for (int i = 0; i < numMatrices; i++)
				DirectX::XMStoreFloat4x4(&projection[i] ,_projection[i]);
			shadowMap = _region;
			light = _light;
		}
	};
	using RegularLight = Light;
	
	
	class PISTACHIO_API Renderer {
	public:
		static void Shutdown();
		static void ChangeSkybox(const char* filename);
		static void Init(const char* skybox);
		static void EndScene();
		static void Submit(Mesh* mesh, Shader* shader,  Material* mat, int ID);
		static void FreeVertexBuffer(const RendererVBHandle handle);
		static void FreeIndexBuffer(const RendererIBHandle handle);
		static void PartialCBUpdate(RendererCBHandle, void* data,uint32_t offset, uint32_t size);
		static void FullCBUpdate(RendererCBHandle handle,void* data);
		static const RendererVBHandle AllocateVertexBuffer(uint32_t size, const void* initialData=nullptr);
		static const RendererCBHandle AllocateConstantBuffer(uint32_t size);
		static const RendererIBHandle AllocateIndexBuffer(uint32_t size,  const void* initialData=nullptr);
		static ComputeShader* GetBuiltinComputeShader(const std::string& name);
		static Shader* GetBuiltinShader(const std::string& name);
		//todo: remove these
		static const uint32_t GetIBOffset(const RendererIBHandle handle);
		static const uint32_t GetVBOffset(const RendererVBHandle handle);
		static const uint32_t GetCBOffset(const RendererCBHandle handle);
		static void  Submit(RHI::Weak<RHI::GraphicsCommandList> list, const RendererVBHandle vb, const RendererIBHandle ib, uint32_t vertexStride);
		static const Texture2D& GetWhiteTexture();
		static RenderCubeMap& GetSkybox();
		static SamplerHandle GetDefaultSampler();
		static const RHI::Ptr<RHI::DynamicDescriptor> GetCBDesc();
		static const RHI::Ptr<RHI::DynamicDescriptor> GetCBDescPS();
		static uint32_t GetCounterValue();
		static void ResetCounter();
		static RHI::Ptr<RHI::Buffer> GetVertexBuffer();
		static RHI::Ptr<RHI::Buffer> GetIndexBuffer();
		static RHI::Ptr<RHI::Buffer> GetConstantBuffer();
		static void OnEvent(Event& e) {
			if (e.GetEventType() == EventType::WindowResize)
				OnWindowResize((WindowResizeEvent&)e);
		}
		static void OnWindowResize(WindowResizeEvent& e)
		{
			
		}
		
	private:
		static void GrowMeshBuffer(uint32_t minExtraSize,
			RHI::BufferUsage usage,
			MonolithicBuffer& buffer);
		static void GrowMeshIndexBuffer(uint32_t minExtraSize);
		static void GrowConstantBuffer(uint32_t minExtraSize);

		static void ChangeRGTexture(RGTextureHandle& texture, RHI::ResourceLayout newLayout, RHI::ResourceAcessFlags newAccess,RHI::QueueFamily newFamily);
		static void DefragmentMeshBuffer(MonolithicBuffer& buffer);
		static void DefragmentConstantBuffer();
	private:
		static RendererContext ctx;
		//-----------OLD-STUFF---------------
		/*
		static DirectX::XMMATRIX viewproj;
		static DirectX::XMVECTOR m_campos;
		static struct CamerData { DirectX::XMMATRIX viewProjection; DirectX::XMMATRIX view;  DirectX::XMFLOAT4 viewPos; }CameraData;
		static Texture2D whiteTexture;
		static std::vector<RegularLight> RegularLightData;
		static std::vector<ShadowCastingLight> ShadowLightData;
		static std::vector<std::uint8_t> LightSBCPU;
		static Material* currentMat;
		static Shader* currentShader;
		static DepthTexture shadowMapAtlas;
		friend class Scene;
		friend class Material;
		static Shader* brdfShader;
		*/
	};
}
