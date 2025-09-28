using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Threading;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using EuclidApp.Interop;
using EuclidApp.Models;
using EuclidApp.Models.Shapes;
using EuclidApp.Utils;
using EuclidApp.Utils.Scaling;
using EuclidApp.Views;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using static EuclidApp.Utils.ScaleMath;
using System.Threading.Tasks;

namespace EuclidApp.ViewModels
{
    public sealed partial class MainViewModel : ViewModelBase
    {
        private EuclidView? _engine;                     
        private readonly Dictionary<ulong, SceneNode> _index = new(); 
        private readonly SceneNode _dummy = new("No selection") { Id = 0 };

        // антизацикливание VM<->Engine
        private readonly ReentrancyGuard _fromVmToEngine = new();
        private readonly ReentrancyGuard _fromEngineToVm = new();

        public ObservableCollection<SceneNode> Scene { get; } = new();

        [ObservableProperty] private bool isPromptOpen;
        [ObservableProperty] private bool isSelectMode = true;

        [ObservableProperty] private SceneNode? selected;


        private static readonly IReadOnlyDictionary<EuclidShapeType, IShapeScaler> s_scalers =
            new Dictionary<EuclidShapeType, IShapeScaler>
            {
        { EuclidShapeType.EUCLID_SHAPE_CUBE,     new CubeScaler()     },
        { EuclidShapeType.EUCLID_SHAPE_SPHERE,   new SphereScaler()   },
        { EuclidShapeType.EUCLID_SHAPE_PLANE,    new PlaneScaler()    },
        { EuclidShapeType.EUCLID_SHAPE_CONE,     new ConeScaler()     },
        { EuclidShapeType.EUCLID_SHAPE_CYLINDER, new CylinderScaler() },
        { EuclidShapeType.EUCLID_SHAPE_PRISM,    new PrismScaler()    },
        { EuclidShapeType.EUCLID_SHAPE_CIRCLE,   new CircleScaler()   },
        { EuclidShapeType.EUCLID_SHAPE_TORUS,    new TorusScaler()    },
            };

        partial void OnSelectedChanged(SceneNode? value)
        {
            OnPropertyChanged(nameof(SelectedSafe));

            if (_engine is null) return;

            var id = value?.Id ?? 0;
            _engine.SetSelection(id);

            if (id != 0 && value is not null)
                SyncNodeTransformFromEngine(value);
        }

        public SceneNode SelectedSafe => Selected ?? _dummy;

        public void AttachEngine(EuclidView engine) => _engine = engine;

        public void OnEngineSelectionChanged(ulong id)
        {
            if (!Dispatcher.UIThread.CheckAccess())
            {
                Dispatcher.UIThread.Post(() => OnEngineSelectionChanged(id));
                return;
            }

            if (id == 0) { Selected = null; return; }
            if (_index.TryGetValue(id, out var node))
                Selected = node;
        }

        private SceneNode EnsureRoot()
        {
            if (Scene.Count == 0 || Scene[0].Name != "Scene")
            {
                SceneGraphUtils.UnhookAll(Scene);
                Scene.Clear();
                _index.Clear();
                Scene.Add(new SceneNode("Scene"));
            }
            return Scene[0];
        }

        private SceneNode AddLeaf(string name, ulong id)
        {
            var root = EnsureRoot();
            var leaf = new SceneNode(name) { Id = id };
            root.Children.Add(leaf);
            _index[id] = leaf;

            HookTransformWriteback(leaf);
            HookParams(leaf);

            SyncNodeTransformFromEngine(leaf);
            return leaf;
        }

        private void RemoveById(ulong id)
        {
            if (id == 0) return;
            foreach (var root in Scene)
            {
                if (root.RemoveById(id))
                {
                    _index.Remove(id);
                    return;
                }
            }
        }

        private void ClearAll()
        {
            SceneGraphUtils.UnhookAll(Scene);
            _index.Clear();
            Scene.Clear();
            Selected = null;
        }

        private Window? GetTopWindow()
        {
            if (Application.Current?.ApplicationLifetime is not IClassicDesktopStyleApplicationLifetime life)
                return null;

            foreach (var w in life.Windows)
                if (w.IsActive)
                    return w;

            return life.MainWindow ?? (life.Windows.Count > 0 ? life.Windows[0] : null);
        }

        [RelayCommand] private void Undo() { /* TODO */ }
        [RelayCommand] private void Redo() { /* TODO */ }

        [RelayCommand] private void Select() => IsSelectMode = true;
        [RelayCommand] private void Translate() => _engine?.SetGizmo(EuclidGizmoMode.EUCLID_GIZMO_TRANSLATE);
        [RelayCommand] private void Rotate() => _engine?.SetGizmo(EuclidGizmoMode.EUCLID_GIZMO_ROTATE);
        [RelayCommand] private void Scale() => _engine?.SetGizmo(EuclidGizmoMode.EUCLID_GIZMO_SCALE);

        [RelayCommand] private void Camera() { }
        [RelayCommand] private void Light() { }
        [RelayCommand] private void RenderSettings() { }
        [RelayCommand] private void GridSettings() { }
        [RelayCommand] private void SceneSettings() { }

        [RelayCommand] private void AiSketch() => IsPromptOpen = true;

        [RelayCommand]
        private async Task OpenRenderDialogAsync(Window? owner)
        {
            var win = new RenderImageWindow
            {
                Width = 1280,
                Height = 800,
                WindowStartupLocation = WindowStartupLocation.CenterOwner
            };

            if (owner is not null) await win.ShowDialog(owner);
            else win.Show();
        }

        [RelayCommand]
        private void AddCube()
        {
            if (_engine is null) return;
            var id = _engine.CreateCube(1.0f);
            if (id == 0) return;

            var leaf = AddLeaf($"Cube {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_CUBE;
            leaf.Params = new CubeParamsVM { Edge = 1.0 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddSphere()
        {
            if (_engine is null) return;
            var id = _engine.CreateSphere(0.5f, 24, 24);
            if (id == 0) return;

            var leaf = AddLeaf($"Sphere {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_SPHERE;
            leaf.Params = new SphereParamsVM { Radius = 0.5, Slices = 24, Stacks = 24 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddTorus()
        {
            if (_engine is null) return;
            var id = _engine.CreateTorus(0.7f, 0.25f, 32, 16);
            if (id == 0) return;

            var leaf = AddLeaf($"Torus {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_TORUS;
            leaf.Params = new TorusParamsVM { MajorRadius = 0.7, MinorRadius = 0.25, SegU = 32, SegV = 16 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddPlane()
        {
            if (_engine is null) return;
            var id = _engine.CreatePlane(1.0f, 1.0f);
            if (id == 0) return;

            var leaf = AddLeaf($"Plane {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_PLANE;
            leaf.Params = new PlaneParamsVM { Width = 1.0, Height = 1.0 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddCone()
        {
            if (_engine is null) return;
            var id = _engine.CreateCone(0.5f, 1.0f, 32);
            if (id == 0) return;

            var leaf = AddLeaf($"Cone {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_CONE;
            leaf.Params = new ConeParamsVM { Radius = 0.5, Height = 1.0, Segments = 32 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddCylinder()
        {
            if (_engine is null) return;
            var id = _engine.CreateCylinder(0.5f, 1.0f, 32);
            if (id == 0) return;

            var leaf = AddLeaf($"Cylinder {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_CYLINDER;
            leaf.Params = new CylinderParamsVM { Radius = 0.5, Height = 1.0, Segments = 32 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddPrism()
        {
            if (_engine is null) return;
            var id = _engine.CreatePrism(6, 0.5f, 1.0f);
            if (id == 0) return;

            var leaf = AddLeaf($"Prism {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_PRISM;
            leaf.Params = new PrismParamsVM { Sides = 6, Radius = 0.5, Height = 1.0 };
            Selected = leaf;
        }

        [RelayCommand]
        private void AddCircle()
        {
            if (_engine is null) return;
            var id = _engine.CreateCircle(0.5f, 64);
            if (id == 0) return;

            var leaf = AddLeaf($"Circle {id}", id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_CIRCLE;
            leaf.Params = new CircleParamsVM { Radius = 0.5, Segments = 64 };
            Selected = leaf;
        }

        // === Import OBJ ===
        [RelayCommand]
        private async Task ImportObjAsync()
        {
            if (_engine is null) return;

            var dlg = new OpenFileDialog
            {
                AllowMultiple = false,
                Filters =
                {
                    new FileDialogFilter { Name = "Wavefront OBJ", Extensions = { "obj" } },
                    new FileDialogFilter { Name = "All files",     Extensions = { "*"   } },
                }
            };

            var top = GetTopWindow();
            var files = await dlg.ShowAsync(top);
            if (files is null || files.Length == 0) return;

            var id = await _engine.ImportObjAsync(files[0], normalize: true);
            if (id == 0) return;

            var leaf = AddLeaf(System.IO.Path.GetFileName(files[0]), id);
            leaf.ShapeType = EuclidShapeType.EUCLID_SHAPE_CUSTOM;
            leaf.Params = new CustomParamsVM();
            Selected = leaf;
        }

        // === Delete / Clear ===
        [RelayCommand]
        private async Task ClearScene()
        {
            if (_engine is null) return;
            await _engine.ClearSceneAsync();
            ClearAll();
        }

        [RelayCommand]
        private async Task DeleteSelected()
        {
            if (_engine is null) return;
            var ok = await _engine.DeleteSelectedAsync();
            if (!ok) return;

            var selId = Selected?.Id ?? 0;
            if (selId != 0)
            {
                RemoveById(selId);
                if (Selected?.Id == selId) Selected = null;
            }
        }

        public void HandleTrellisSubmit(string prompt, AttachmentItem[]? images)
        {
            Console.WriteLine($"Trellis prompt: {prompt} | images: {images?.Length ?? 0}");
            IsPromptOpen = false;
        }

        // ===== Transform sync helpers =====
        private void HookTransformWriteback(SceneNode node)
        {
            node.__transformHandler ??= OnNodeTransformChanged;
            node.Transform.PropertyChanged -= node.__transformHandler;
            node.Transform.PropertyChanged += node.__transformHandler;
        }

        private void OnNodeTransformChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (_engine is null || Selected is null || Selected.Id == 0) return;
            if (_fromEngineToVm.IsEntered) return; 

            using var _ = _fromVmToEngine.Enter();

            var t = Selected.Transform;
            var tf = new EuclidTransform
            {
                posX = (float)t.PosX,
                posY = (float)t.PosY,
                posZ = (float)t.PosZ,
                rotX = (float)t.RotX,
                rotY = (float)t.RotY,
                rotZ = (float)t.RotZ,
                sclX = (float)t.SclX,
                sclY = (float)t.SclY,
                sclZ = (float)t.SclZ
            };

            _engine.TrySetTransform(Selected.Id, in tf);
        }

        private void SyncNodeTransformFromEngine(SceneNode node)
        {
            if (_engine is null || node.Id == 0) return;
            if (_fromVmToEngine.IsEntered) return;

            if (_engine.TryGetTransform(node.Id, out var tf))
            {
                using var _ = _fromEngineToVm.Enter();
                var tr = node.Transform;
                tr.PosX = tf.posX; tr.PosY = tf.posY; tr.PosZ = tf.posZ;
                tr.RotX = tf.rotX; tr.RotY = tf.rotY; tr.RotZ = tf.rotZ;
                tr.SclX = tf.sclX; tr.SclY = tf.sclY; tr.SclZ = tf.sclZ;
            }
        }

        public void OnEngineTransformPolled(ulong id, EuclidTransform tf)
        {
            if (!Dispatcher.UIThread.CheckAccess())
            {
                Dispatcher.UIThread.Post(() => OnEngineTransformPolled(id, tf));
                return;
            }

            var node = Selected;
            if (node is null || node.Id != id || node.IsFolder) return;
            if (_fromVmToEngine.IsEntered) return;

            using var _ = _fromEngineToVm.Enter();

            var t = node.Transform;
            const double eps = 1e-6;

            if (Math.Abs(t.PosX - tf.posX) > eps) t.PosX = tf.posX;
            if (Math.Abs(t.PosY - tf.posY) > eps) t.PosY = tf.posY;
            if (Math.Abs(t.PosZ - tf.posZ) > eps) t.PosZ = tf.posZ;

            if (Math.Abs(t.RotX - tf.rotX) > eps) t.RotX = tf.rotX;
            if (Math.Abs(t.RotY - tf.rotY) > eps) t.RotY = tf.rotY;
            if (Math.Abs(t.RotZ - tf.rotZ) > eps) t.RotZ = tf.rotZ;

            if (Math.Abs(t.SclX - tf.sclX) > eps) t.SclX = tf.sclX;
            if (Math.Abs(t.SclY - tf.sclY) > eps) t.SclY = tf.sclY;
            if (Math.Abs(t.SclZ - tf.sclZ) > eps) t.SclZ = tf.sclZ;
        }

        private void HookParams(SceneNode node)
        {
            if (node.Params is null) return;

            node.__paramsHandler ??= Params_PropertyChanged;
            node.Params.PropertyChanged -= node.__paramsHandler;
            node.Params.PropertyChanged += node.__paramsHandler;

            UpdateScaleFromParams(node); 
        }

        private void Params_PropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (Selected is null || sender != Selected.Params) return;
            UpdateScaleFromParams(Selected);
        }

        private void UpdateScaleFromParams(SceneNode node)
        {
            if (_engine is null || node.Id == 0) return;
            if (node.ShapeType is null) return;

            var keyNow = Utils.SceneGraphUtils.BuildParamKey(node);
            if (node.__lastParamKey is not null && node.__lastParamKey == keyNow)
                return;

            if (!s_scalers.TryGetValue(node.ShapeType.Value, out var scaler))
                return;

            if (!scaler.TryScale(node, out var desired))
            {
                node.__lastParamKey = keyNow;
                return;
            }

            if (!_engine.TryGetTransform(node.Id, out var tf))
            {
                node.__lastParamKey = keyNow;
                return;
            }

            var current = new Vec3f(tf.sclX, tf.sclY, tf.sclZ);

            if (current.NearlyEquals(desired))
            {
                node.__lastParamKey = keyNow;
                node.__lastAutoScale = current;
                return;
            }

            tf.sclX = ClampPos(desired.X);
            tf.sclY = ClampPos(desired.Y);
            tf.sclZ = ClampPos(desired.Z);

            using var _ = _fromVmToEngine.Enter();
            _engine.TrySetTransform(node.Id, in tf);

            node.__lastParamKey = keyNow;
            node.__lastAutoScale = new Vec3f(tf.sclX, tf.sclY, tf.sclZ);
        }

    }
}
