using Avalonia.Controls;
using Avalonia.Media;
using System.Diagnostics;

namespace EditorCSharp.Views;

public partial class MainView : UserControl
{
    public MainView()
    {
        InitializeComponent();
        this.DataContextChanged += MainView_DataContextChanged;
    }

    private void MainView_DataContextChanged(object? sender, System.EventArgs e)
    {
        var DataContext = this.DataContext as ViewModels.MainViewModel;
        var svControl = this.GetControl<EditorCSharp.Controls.SceneView>("Scene");
        svControl.app = DataContext.application;
        svControl.scene = DataContext.scene;
        svControl.camera = DataContext.editorCamera;
    }
}
