#include "Pistachio/Scene/Entity.h"
#include "Pistachio/Scene/Scene.h"
#include "angelscript.h"
#include "ptpch.h"
#include "ScriptAPICommon.h"
#include "scriptarray.h"
#include <fstream>
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
            void Scene_AddRef(Scene* scene)
            {
                scene_ref_count[scene]++;
            }
            void Scene_Release(Scene* scene)
            {
                int newRef = --scene_ref_count[scene];
                if(!newRef)
                {
                    delete scene;
                }
            }
            CScriptArray* GetAllEntitiesWithName(Scene* scene, const std::string& name)
            {
                auto entities = scene->GetAllEntitesWithName(name);
                auto arr = CScriptArray::Create(entityType, entities.size());
                if(entities.size()) memcpy(arr->GetBuffer(), entities.data(), sizeof(Entity) * entities.size());
                return arr;
            }
            void Initialize()
            {
                engine->SetDefaultNamespace("Pistachio::ECS");
                engine->RegisterObjectType("Entity", sizeof(Entity), asOBJ_VALUE | asGetTypeTraits<Entity>());
                engine->RegisterObjectType("Scene", 0, asOBJ_REF);

                engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "Entity (const Entity& other)", asMETHOD(Entity, operator=), asCALL_THISCALL);
                engine->RegisterObjectBehaviour("Scene", asBEHAVE_ADDREF, "void f()", asFUNCTION(Scene_AddRef), asCALL_CDECL_OBJFIRST);
                engine->RegisterObjectBehaviour("Scene", asBEHAVE_RELEASE, "void f()", asFUNCTION(Scene_Release), asCALL_CDECL_OBJFIRST);

                engine->RegisterObjectType("Transform", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("MeshRenderer", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("Camera", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("Light", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("RigidBody", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("SphereCollider", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("BoxCollider", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("CapsuleCollider", 0, asOBJ_REF | asOBJ_NOCOUNT);
                engine->RegisterObjectType("PlaneCollider", 0, asOBJ_REF | asOBJ_NOCOUNT);

                engine->RegisterObjectMethod("Scene", "Entity CreateEntity(const string& name)", asMETHOD(Scene, CreateEntity), asCALL_THISCALL);
                engine->RegisterObjectMethod("Scene", "void DestroyEntity(Entity e)", asMETHOD(Scene, DefferedDelete),asCALL_THISCALL);
                engine->RegisterObjectMethod("Scene", "Entity GetEntityByUUID(UUID uuid)", asMETHOD(Scene, GetEntityByUUID), asCALL_THISCALL);
                engine->RegisterObjectMethod("Scene", "Entity GetEntityByName(const string& name)", asMETHOD(Scene, GetEntityByName), asCALL_THISCALL);
                engine->RegisterObjectMethod("Scene", "array<Entity>@ GetAllEntitiesWithName(const string& name)", asFUNCTION(ECS::GetAllEntitiesWithName), asCALL_CDECL_OBJFIRST);

                //TODO: GetComponenet

                engine->RegisterGlobalProperty("Entity current_entity", &current_e);
                engine->RegisterGlobalProperty("Scene@ current_entity", &current_s);
                engine->SetDefaultNamespace("");
            }
        }
    }
}