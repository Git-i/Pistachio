# ECS Module

The entire module is exposed under the `Pistachio::ECS` namespace

## Types

[`Entity`](Entity.md)

[`Scene`](Scene.md)

[`Transform`](Transform.md)

[`MeshRenderer`](MeshRenderer.md)

[`Light`](Light.md)

## Properties

```csharp
const Scene@ current_scene;
```

This represents the active scene while the script is executing.

---

```csharp
const Entity current_entity
```

This is the entity that owns the currently running script.
