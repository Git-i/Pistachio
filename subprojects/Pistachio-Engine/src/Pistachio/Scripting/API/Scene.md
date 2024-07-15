# Scene

Scene is a type representing a scene in Pistachio.

Module: [ECS](ECS.md)

Namespace: `Pistachio::ECS`

## Constructors

The Scene is not a constructable type, you retireve the current instance using the global provided by the [ECS](ECS.md) module.

## Member Functions

#### CreateEntity

```csharp
Entity CreateEntity(const string &in name)
```

Creates an entity with a specific name. Entity names do not have to be unique.

###### Parameters

- name: The name of the new entity

###### Return Value

        The newly created entity

#### DestroyEntity

```csharp
void DestroyEntity(Entity e)
```

Destorys the specified entity. It is perfectly safe to destroy the current entity, even when the script is executing, as entity deletions are queued and only happen at the end of a frame.

###### Parameters

- e: The identifier of the entity to be destoryed

#### GetEntityByUUID

```csharp
Entity GetEntityByUUID(UUID uuid)
```

###### Parameters

- uuid: The UUID of the entity to retrieve

###### Return Value

        The Entity with specified entity, or an invalid entity if not found.

#### GetEntityByName

```csharp
Entity GetEntityByName(const string &in name)
```

Retrieves an entity with the specified name, since multiple entities can share a name, it is recommended you retrieve by UUID.

###### Parameters

- name: The name of the entity to retrieve

###### Return Value

        An entity with the specified name, or an invalid entity if none were found

#### GetAllEntitiesWithName

```csharp
array<Entity>@ GetAllEntitiesWithName(const string &in name)
```

Retrieves all entities with names matching the specified name

###### Parameters

- name: The name of entities to retireive

###### Return Value

        An array of all entities with the specified name.
