// ViewModels/MainViewModel.cs
using Avalonia.Styling;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;
using System.Net.Sockets;

namespace EuclidApp.ViewModels
{
    public sealed partial class MainViewModel : ViewModelBase
    {
        public ObservableCollection<SceneNode> Scene { get; } = new();

        [ObservableProperty]
        private SceneNode? selected;

        [ObservableProperty]
        private bool isSelectMode = true;

        public MainViewModel()
        {
            var pkg1 = new SceneNode("Package1");
            pkg1.Children.Add(new SceneNode("Cube.obj"));
            var pkg2 = new SceneNode("Package2");
            pkg2.Children.Add(new SceneNode("Skybox.obj"));
            Scene.Add(new SceneNode("Scene", new[] { pkg1, pkg2 }));

            Selected = pkg1.Children[0];
        }

        [RelayCommand] private void Undo() { }
        [RelayCommand] private void Redo() { }
        [RelayCommand] private void Select() => IsSelectMode = true;
        [RelayCommand] private void Translate() => IsSelectMode = false;
        [RelayCommand] private void Rotate() { }
        [RelayCommand] private void Scale() { }
        [RelayCommand] private void AiSketch() { }
        [RelayCommand] private void Camera() { }
        [RelayCommand] private void Light() { }
        [RelayCommand] private void RenderSettings() { }
        [RelayCommand] private void GridSettings() { }
        [RelayCommand] private void SceneSettings() { }
    }

    public sealed partial class SceneNode : ObservableObject
    {
        public string Name { get; }
        public ObservableCollection<SceneNode> Children { get; } = new();
        public Transform Transform { get; } = new();

        public SceneNode(string name) => Name = name;
        public SceneNode(string name, System.Collections.Generic.IEnumerable<SceneNode> children) : this(name)
        {
            foreach (var c in children) Children.Add(c);
        }
    }

    public sealed partial class Transform : ObservableObject
    {
        // Location
        [ObservableProperty] private double posX;
        [ObservableProperty] private double posY;
        [ObservableProperty] private double posZ;

        // Rotation (в градусах)
        [ObservableProperty] private double rotX; 
        [ObservableProperty] private double rotY;
        [ObservableProperty] private double rotZ; 

        // Scale
        [ObservableProperty] private double sclX = 1.0;
        [ObservableProperty] private double sclY = 1.0;
        [ObservableProperty] private double sclZ = 1.0;
    }
}
