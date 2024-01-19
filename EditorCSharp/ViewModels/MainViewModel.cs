using System;
using System.Collections.ObjectModel;

namespace EditorCSharp.ViewModels;
public class EntityNode
{
    public Entity entity;
    public Scene scene;
    public string? tag { get; }
    public ObservableCollection<EntityNode>? children { get; }
    public void DeleteEntity()
    {
        scene.DeleteEntity(entity);
    }
    public EntityNode(Entity entity, Scene scene)
    {
        this.entity = entity;
        this.scene = scene;
        tag = entity.GetComponent<TagComponent>().Tag;
        children = new ObservableCollection<EntityNode>();
        var entityChildren = scene.GetEntityChildern(entity);
        if (entityChildren != null)
        {
            foreach (var child in entityChildren)
            {
                children.Add(new EntityNode(child, scene));
            }
        }
    }
}
class EditorLayer : PistachioCS.Layer
{
    public override void OnAttach()
    {
        Console.WriteLine("Attach");

        IDComponent id = entity.GetComponent<IDComponent>();
        MeshRendererComponent mr = entity.AddComponent<MeshRendererComponent>();
        //EditorCamera a = entity.GetComponent<EditorCamera>(); // exception thrown
        Asset asset = AssetManager.CreateModelAsset("assets/models/obj/ceberus.obj");

        asset = AssetManager.CreateModelAsset("assets/models/obj/ceberus.obj");

        mr.Model = asset;
        Console.WriteLine(id.UUID.Id);

    }
    public override void OnDetach()
    {
        Console.WriteLine("Detach");

    }
    public override void OnUpdate(float delta)
    {
        camera.OnUpdate(delta);
        scene.OnUpdateEditor(delta, camera);
        if (entity.IsValid())
        {
            Vector3 rotation = entity.GetComponent<TransformComponent>().Rotation;
            rotation.y += 0.2f * delta * 10.0f;
            entity.GetComponent<TransformComponent>().Rotation = rotation;
        }

    }
    public EditorLayer()
    {
        scene = new PistachioCS.Scene(1280, 720);
        scene.SceneGraphChanged += SceneUpdate;
        camera = new(30.0f, 1.6f, 0.1f, 1000.0f);
        entities = new ObservableCollection<EntityNode>();
        entities.Add(new EntityNode(scene.GetRootEntity(), scene));
        entity = scene.CreateEntity("asd");
        Entity e = scene.CreateEntity("asd's-child");
        scene.ReParentEntity(e, entity);
        Entity b = scene.CreateEntity("asd's-grandchild?");
        scene.ReParentEntity(b, e);

    }
    private void SceneUpdate(object? sender, SceneGraphChangedArgs args)
    {
        //entities.Clear();
        //Entity root = scene.GetRootEntity();
        //entities.Add(new EntityNode(root, scene));
        if (args.op == SceneGraphOp.EntityCreated)
        {
            ObservableCollection<EntityNode> currentCollection = entities[0].children;
            for (int i = args.entityParentHierarchy.Count - 1; i > 0; i--)
            {
                //find index to append entity node
                foreach (EntityNode node in currentCollection)
                {
                    if (node.entity == args.entityParentHierarchy[i])
                    {
                        currentCollection = currentCollection[currentCollection.IndexOf(node)].children;
                    }
                }
            }
            currentCollection.Add(new EntityNode(args.affectedEntity, scene));
        }
        if (args.op == SceneGraphOp.EntityReparented)
        {
            //add it to the new hierarchy
            ObservableCollection<EntityNode> currentCollection = entities[0].children;
            for (int i = args.entityParentHierarchy.Count - 1; i >= 0; i--)
            {
                //find index to append entity node
                foreach (EntityNode node in currentCollection)
                {
                    if (node.entity.op_Equality(args.entityParentHierarchy[i]))
                    {
                        currentCollection = currentCollection[currentCollection.IndexOf(node)].children;
                        break;
                    }
                }
            }
            currentCollection.Add(new EntityNode(args.affectedEntity, scene));
            //remove it to the new hierarchy
            currentCollection = entities[0].children;
            for (int i = args.OldEntityParentHierarchy.Count - 1; i >= 0; i--)
            {
                //find index to append entity node
                foreach (EntityNode node in currentCollection)
                {
                    if (node.entity.op_Equality(args.OldEntityParentHierarchy[i]))
                    {
                        currentCollection = currentCollection[currentCollection.IndexOf(node)].children;
                        break;
                    }
                }
            }
            foreach (EntityNode node in currentCollection)
            {
                if (node.entity.op_Equality(args.affectedEntity))
                {
                    currentCollection.Remove(node);
                    break;
                }
            }
        }
        if (args.op == SceneGraphOp.EntityRemoved)
        {
            var currentCollection = entities[0].children;
            for (int i = args.entityParentHierarchy.Count - 1; i >= 0; i--)
            {
                //find index to append entity node
                foreach (EntityNode node in currentCollection)
                {
                    if (node.entity.op_Equality(args.entityParentHierarchy[i]))
                    {
                        currentCollection = currentCollection[currentCollection.IndexOf(node)].children;
                        break;
                    }
                }
            }
            foreach (EntityNode node in currentCollection)
            {
                if (node.entity.op_Equality(args.affectedEntity))
                {
                    currentCollection.Remove(node);
                    break;
                }
            }
        }
    }
    public PistachioCS.Scene scene;
    public PistachioCS.EditorCamera camera;
    public PistachioCS.Entity entity;
    public ObservableCollection<EntityNode> entities;

}
public class MainViewModel : ViewModelBase
{
    public PistachioCS.Scene scene { get => _layer.scene; }
    public PistachioCS.Application application { get => app; }
    public PistachioCS.EditorCamera editorCamera { get => _layer.camera; }
    public ObservableCollection<EntityNode> entityNodes { get => _layer.entities; }
    private EntityNode? _currentNode;
    public EntityNode? currentNode { set { this.RaiseAndSetIfChanged(ref _currentNode, value); } }
    public MainViewModel()
    {
        app = new("lmao", true);
        _layer = new();
        app.PushLayer(_layer);
        var pixelBuffer = _layer.scene.GetImage();
    }
    public void AddEntity()
    {
        _layer.scene.CreateEntity("New Unnamed Entity");
    }
    private EditorLayer _layer;
    private PistachioCS.Application app;
    public string Greeting => "Welcome to Avalonia!";
}
