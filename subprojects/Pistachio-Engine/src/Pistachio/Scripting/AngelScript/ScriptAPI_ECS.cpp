#include "Pistachio/Renderer/Mesh.h"
#include "Pistachio/Scene/Components.h"
#include "Pistachio/Scene/Entity.h"
#include "Pistachio/Scene/Scene.h"
#include "angelscript.h"
#include "ptpch.h"
#include "ScriptAPICommon.h"
#include "scriptarray.h"
#include <cstring>
#include <fstream>
#include "aswrappedcall.h"
#include <string_view>
#include <unordered_map>
namespace Pistachio
{
    namespace Scripting
    {
        namespace ECS
        {
            asITypeInfo* entityType;
            Entity current_e;
            Scene* current_s = nullptr;
            std::unordered_map<Scene*, int> scene_ref_count; //used to keep track of reference counts for scenes used in script

            /*
            * Every Scene that goes into a script must be refcounted i.e
            * registeres in the ref count map, this is done automatically for scenes created
            * in a script.
            */
            void SetCurrentScene(Scene* scene)
            {
                current_s = scene;
                scene_ref_count[current_s] = 1;
            }
            void Scene_AddRef(Scene* scene)
            {
                scene_ref_count[scene]++;
            }
            void Scene_Release(Scene* scene)
            {
                int newRef = --scene_ref_count[scene];
                if(!newRef)
                {
                    scene_ref_count.erase(scene);
                    delete scene;
                }
            }
            void Entity_GetComponenet(asIScriptGeneric* gen)
            {
                Entity* e = (Entity*)gen->GetObject();
                auto ctx = asGetActiveContext();
                if(!e->IsValid())
                {
                    ctx->SetException("Call to GetComponent with Invalid Entity");
                    return;
                }
                int tid = gen->GetReturnTypeId();
                auto info = engine->GetTypeInfoById(tid);
                const char* typeName = info->GetName();
                std::string_view name = typeName;
                if(name == "Transform")
                    gen->SetReturnAddress(&e->GetComponent<TransformComponent>());
                else if(name == "MeshRenderer")
                    gen->SetReturnAddress(&e->GetComponent<MeshRendererComponent>());
                else if(name == "Camera")
                    gen->SetReturnAddress(&e->GetComponent<CameraComponent>());
                else if(name == "Light")
                    gen->SetReturnAddress(&e->GetComponent<LightComponent>());
                else if(name == "RigidBody")
                    gen->SetReturnAddress(&e->GetComponent<RigidBodyComponent>());
                else if(name == "SphereCollider")
                    gen->SetReturnAddress(&e->GetComponent<SphereColliderComponent>());
                else if(name == "BoxCollider")
                    gen->SetReturnAddress(&e->GetComponent<BoxColliderComponent>());
                else if(name == "CapsuleCollider")
                    gen->SetReturnAddress(&e->GetComponent<CapsuleColliderComponent>());
                else if(name == "PlaneCollider")
                    gen->SetReturnAddress(&e->GetComponent<PlaneColliderComponent>());
                else
                {
                    char msg[1024];
                    strcpy(msg, typeName);
                    strcat(msg, " is not a valid Componenet Type");
                    ctx->SetException(msg);
                }

            }
            void Entity_Construct(void* memory)
            {
                new(memory) Entity(entt::null, current_s);
            }
            void Entity_Copy(const Entity& other, Entity* current)
            {
                *current = Entity(other);
                DEBUG_BREAK;
            }
            void Entity_Destruct(void* memory)
            {
            }
            CScriptArray* GetAllEntitiesWithName(Scene* scene, const std::string& name)
            {
                auto entities = scene->GetAllEntitesWithName(name);
                auto arr = CScriptArray::Create(engine->GetTypeInfoByDecl("array<Pistachio::ECS::Entity>"), entities.size());
                arr->Reserve(entities.size());
                uint32_t size = arr->GetSize();
                for(uint32_t i = 0; i < size; i++)
                {
                    arr->InsertLast(&entities[i]);
                }
                return arr;
            }
            void Initialize()
            {
                int r;
                r = engine->SetDefaultNamespace("Pistachio::ECS"); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("Entity", sizeof(Entity), asOBJ_VALUE|asGetTypeTraits<Entity>()); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("Scene", 0, asOBJ_REF); PT_CORE_ASSERT(r >= 0);

                entityType = engine->GetTypeInfoByDecl("Entity");

                r = engine->RegisterObjectMethod("Entity", "void opAssign(const Entity &in e)", asFUNCTION(Entity_Copy), asCALL_CDECL_OBJLAST);
                r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Entity_Construct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f(const Entity& in)", asFUNCTION(Entity_Copy), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Entity_Destruct), asCALL_CDECL_OBJLAST); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectBehaviour("Scene", asBEHAVE_ADDREF, "void f()", asFUNCTION(Scene_AddRef), asCALL_CDECL_OBJFIRST); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectBehaviour("Scene", asBEHAVE_RELEASE, "void f()", asFUNCTION(Scene_Release), asCALL_CDECL_OBJFIRST); PT_CORE_ASSERT(r >= 0);

                r = engine->RegisterObjectType("Transform", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("MeshRenderer", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("Camera", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("Light", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("RigidBody", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("SphereCollider", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("BoxCollider", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("CapsuleCollider", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectType("PlaneCollider", 0, asOBJ_REF | asOBJ_NOCOUNT); PT_CORE_ASSERT(r >= 0);
                
                r = engine->RegisterObjectMethod("Scene", "Entity CreateEntity(const string &in name)", WRAP_MFN(Scene, CreateEntity), asCALL_GENERIC); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectMethod("Scene", "void DestroyEntity(Entity e)", WRAP_MFN(Scene, DefferedDelete),asCALL_GENERIC); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectMethod("Scene", "Entity GetEntityByUUID(UUID uuid)", WRAP_MFN(Scene, GetEntityByUUID), asCALL_GENERIC); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectMethod("Scene", "Entity GetEntityByName(const string &in name)", WRAP_MFN(Scene, GetEntityByName), asCALL_GENERIC); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterObjectMethod("Scene", "array<Entity>@ GetAllEntitiesWithName(const string &in name)", asFUNCTION(ECS::GetAllEntitiesWithName), asCALL_CDECL_OBJFIRST); PT_CORE_ASSERT(r >= 0);

                //TODO: GetComponenet
                
                r = engine->RegisterGlobalProperty("Entity current_entity", &current_e); PT_CORE_ASSERT(r >= 0);
                r = engine->RegisterGlobalProperty("Scene@ current_scene", &current_s); PT_CORE_ASSERT(r >= 0);
                r = engine->SetDefaultNamespace(""); PT_CORE_ASSERT(r >= 0);
            }
        }
    }
}