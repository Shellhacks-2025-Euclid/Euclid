using CommunityToolkit.Mvvm.ComponentModel;
using EuclidApp.Interop;
using EuclidApp.Utils;
using System.Collections.ObjectModel;
using System.ComponentModel;

namespace EuclidApp.Models
{
    public sealed partial class SceneNode : ObservableObject
    {
        public string Name { get; }
        public ObservableCollection<SceneNode> Children { get; } = new();

        [ObservableProperty] private ulong id;

        [ObservableProperty] private EuclidShapeType? shapeType;
        [ObservableProperty] private Shapes.ShapeParamsVM? @params;

        internal Vec3f? __lastAutoScale;  
        internal string? __lastParamKey; 

        public Transform Transform { get; } = new();

        internal PropertyChangedEventHandler? __transformHandler;
        internal PropertyChangedEventHandler? __paramsHandler;

        public SceneNode(string name)
        {
            Name = name;
            Children.CollectionChanged += (_, __) =>
            {
                OnPropertyChanged(nameof(HasChildren));
                OnPropertyChanged(nameof(IsFolder));
                OnPropertyChanged(nameof(IsLeaf));
            };
        }

        public SceneNode(string name, System.Collections.Generic.IEnumerable<SceneNode> children) : this(name)
        {
            foreach (var c in children) Children.Add(c);
        }

        public bool HasChildren => Children.Count > 0;
        public bool IsFolder => HasChildren || Id == 0;
        public bool IsLeaf => !IsFolder;

        public SceneNode? FindById(ulong id)
        {
            if (Id == id) return this;
            foreach (var c in Children)
            {
                var f = c.FindById(id);
                if (f is not null) return f;
            }
            return null;
        }

        public bool RemoveById(ulong id)
        {
            for (int i = 0; i < Children.Count; i++)
            {
                if (Children[i].Id == id)
                {
                    Utils.SceneGraphUtils.UnhookAll(Children[i]);
                    Children.RemoveAt(i);
                    return true;
                }
                if (Children[i].RemoveById(id))
                    return true;
            }
            return false;
        }
    }
}
