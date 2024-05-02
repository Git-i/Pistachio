using Avalonia;
using Avalonia.Controls;
using Avalonia.Rendering.Composition;
using Avalonia.VisualTree;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorCSharp.Vulkan
{
    
    internal class ImageRenderer : UserControl
    {
        
        private CompositionSurfaceVisual? _visual;
        private Compositor? _compositor;
        private readonly Action _update;
        protected CompositionDrawingSurface? Surface { get; private set; }
        public ImageRenderer()
        {
            _update = UpdateFrame;
        }
        void UpdateFrame()
        {
        }
        void Draw()
        {
            _compositor.RequestCompositionUpdate(_update);
        }
    }
}
