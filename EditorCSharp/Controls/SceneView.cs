using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorCSharp.Controls
{
    public class SceneView : Control
    {
        public PistachioCS.Application? app { get; set; }
        public PistachioCS.Scene? scene { get; set; }
        public PistachioCS.EditorCamera? camera { get; set; }
        private Avalonia.Size size = new Avalonia.Size(0, 0);
        private WriteableBitmap Source;

        public SceneView()
        {
            Source = new(new Avalonia.PixelSize(1280, 720), new Avalonia.Vector(96, 96), Avalonia.Platform.PixelFormat.Rgba8888, Avalonia.Platform.AlphaFormat.Opaque);
        }

        public override void Render(DrawingContext context)
        {
            if (app != null && scene != null)
            {
                app.Run();
                nint pixelbuffer = scene.GetImage();
                var buffer = Source.Lock();
                unsafe
                {
                    Buffer.MemoryCopy((void*)pixelbuffer, (void*)buffer.Address, 1280 * 720 * 4, 1280 * 720 * 4);
                }
                buffer.Dispose();
                scene.FreeImage(pixelbuffer);
                if (camera != null)
                {
                    if (Bounds.Size != size) { size = Bounds.Size; camera.UpdateViewportSize((float)size.Width, (float)size.Height); }
                }
                context.DrawImage(Source, new Avalonia.Rect(Bounds.Size));
            }
            base.Render(context);
            Dispatcher.UIThread.InvokeAsync(InvalidateVisual, DispatcherPriority.Background);
        }
    }
}
