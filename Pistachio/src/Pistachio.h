#pragma once
#include "ptpch.h"
#include "Pistachio/Core/Application.h"
#include "Pistachio/Core/LayerStack.h"
#include "Pistachio/Event/Event.h"
#include "Pistachio/Event/ApplicationEvent.h"
#include "Pistachio/Event/MouseEvent.h"
#include "Pistachio/Core/Transform.h"
#include "Pistachio/Renderer/Texture.h"
#include "Pistachio/Renderer/Sampler.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Input.h"
#include "Pistachio/Core/KeyCodes.h"
#include "Pistachio/Renderer/Renderer.h"
#include "Pistachio/Renderer/Mesh.h"
#include "Pistachio/Renderer/RenderTexture.h"
#include "Pistachio/Renderer/Renderer2D.h"
#include "Pistachio/Renderer/Model.h"
#include "Pistachio/Scene/Scene.h"
#include "Pistachio/Scene/Components.h"
#include "Pistachio/Scene/Entity.h"
#include "Pistachio/Scene/ScriptableComponent.h"
#include "Pistachio/Renderer/FrameComposer.h"
//necessary for agility sdk
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 611; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
