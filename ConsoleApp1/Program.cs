// See https://aka.ms/new-console-template for more information
class EditorLayer : PistachioCS.Layer
{
    public override void OnAttach()
    {
        Console.WriteLine("Attach");
        PistachioCS.Entity entity = scene.CreateEntity("asd");
        IDComponent id = entity.GetComponent<IDComponent>();
        TagComponent tag = entity.GetComponent<TagComponent>();
        MeshRendererComponent mr = entity.AddComponent<MeshRendererComponent>();
        //EditorCamera a = entity.GetComponent<EditorCamera>(); // exception thrown
        Asset asset = AssetManager.CreateModelAsset("assets/models/obj/ceberus.obj");
        TransformComponent transform = entity.AddComponent<TransformComponent>();
        asset = AssetManager.CreateModelAsset("assets/models/obj/ceberus.obj");
        TagComponent tagComponent = entity.AddComponent<TagComponent>();
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
        var buffer = scene.GetImage();
        Console.WriteLine(buffer);

    }
    public EditorLayer()
    {
        scene = new PistachioCS.Scene(1280, 720);
        camera = new(30.0f, 1.6f, 0.1f, 1000.0f);
    }
    private PistachioCS.Scene scene;
    private PistachioCS.EditorCamera camera;

}
class Program
{
    public static void Main(string[] args)
    {
        PistachioCS.Application a = new PistachioCS.Application("5", true);
        EditorLayer layer = new EditorLayer();
        a.PushLayer(layer);
        a.Run();
        Console.WriteLine("Hello, World!");
    }
}
