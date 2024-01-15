
using Avalonia.Media;
using PistachioCS;
using ReactiveUI;
using System;
using System.Reactive;

namespace EditorCSharp.ViewModels;
class EditorLayer : PistachioCS.Layer
{
    public override void OnAttach()
    {
        Console.WriteLine("Attach");
        
        IDComponent id = entity.GetComponent<IDComponent>();
        TagComponent tag = entity.GetComponent<TagComponent>();
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
        Vector3 rotation = entity.GetComponent<TransformComponent>().Rotation;
        rotation.y += 0.2f * delta * 10.0f;
        entity.GetComponent<TransformComponent>().Rotation = rotation;

    }
    public EditorLayer()
    {
        scene = new PistachioCS.Scene(1280, 720);
        camera = new(30.0f, 1.6f, 0.1f, 1000.0f);
        entity = scene.CreateEntity("asd");
    }
    public PistachioCS.Scene scene;
    public PistachioCS.EditorCamera camera;
    public PistachioCS.Entity entity;

}
public class MainViewModel : ViewModelBase
{
    public PistachioCS.Scene scene
    {
        get
        {
            return _layer.scene;
        }
    }
    public PistachioCS.Application application
    {
        get
        {
            return app;
        }
    }
    public PistachioCS.EditorCamera editorCamera
    {
        get
        {
            return _layer.camera;
        }
    }
    public MainViewModel()
    {
        app = new("lmao", true);
        _layer = new();
        app.PushLayer(_layer);
        var pixelBuffer = _layer.scene.GetImage();
    }
    private EditorLayer _layer;
    private PistachioCS.Application app;
    public string Greeting => "Welcome to Avalonia!";
}
