using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Input;
using Avalonia.OpenGL;
using Avalonia.OpenGL.Controls;
using Avalonia.Rendering;
using Avalonia.VisualTree;
using EuclidApp.Interop;

namespace EuclidApp.Views
{
    public sealed class EuclidView : OpenGlControlBase
    {
        private IntPtr _euclid = IntPtr.Zero;
        private readonly Stopwatch _timer = new();
        private double _lastT;
        private int _sentW, _sentH;
        private ulong _lastSelection;

        private bool _dragging;
        private bool _orbitViaCtrlLeft;
        private Point _lastPt; // logical
        private double _virtX; // physical px
        private double _virtY; // physical px
        private const double DragSensitivity = 1.0;

        private readonly ConcurrentQueue<Action> _glJobs = new();

        private bool _pendingPick;
        private float _pendingPickX;
        private float _pendingPickY;

        private bool _lastTfValid;
        private EuclidTransform _lastTf;

        public event Action? EngineReady;
        public event Action<ulong>? SelectionChanged;


        public event Action<ulong, EuclidTransform>? TransformPolled;

        public bool IsReady => _euclid != IntPtr.Zero;

        public EuclidView()
        {
            Focusable = true;
            IsHitTestVisible = true;
            SizeChanged += (_, __) => SendResizeIfNeeded();
        }

        // ===== DPI helpers =====
        private double GetScale()
            => (this.GetVisualRoot() as IRenderRoot)?.RenderScaling ?? 1.0;

        private (int wPx, int hPx) GetPixelSize()
        {
            var s = GetScale();
            var w = Math.Max(1, (int)Math.Round(Bounds.Width * s));
            var h = Math.Max(1, (int)Math.Round(Bounds.Height * s));
            return (w, h);
        }

        private void EnqueueGlJob(Action job)
        {
            _glJobs.Enqueue(job);
            RequestNextFrameRendering();
        }

        // ===== GL lifecycle =====
        protected override void OnOpenGlInit(GlInterface gl)
        {
            var (w, h) = GetPixelSize();

            var cfg = new EuclidConfig
            {
                width = w,
                height = h,
                gl_major = 3,
                gl_minor = 3
            };

            if (EuclidNative.Euclid_Create(ref cfg, gl.GetProcAddress, out _euclid) != 0 || _euclid == IntPtr.Zero)
                return;

            _sentW = _sentH = 0;
            SendResizeIfNeeded();

            _timer.Restart();
            _lastT = 0;
            _lastSelection = 0;

            _pendingPick = false;

            _lastTfValid = false;

            EngineReady?.Invoke();
            RequestNextFrameRendering();
        }

        protected override void OnOpenGlDeinit(GlInterface gl)
        {
            _euclid = IntPtr.Zero;
        }

        protected override void OnOpenGlRender(GlInterface gl, int fb)
        {
            if (_euclid == IntPtr.Zero) return;

            while (_glJobs.TryDequeue(out var job))
            {
                try { job(); } catch {  }
            }

            EuclidNative.Euclid_SetFramebuffer(_euclid, (uint)fb);
            SendResizeIfNeeded();

            var now = _timer.Elapsed.TotalSeconds;
            var dt = (float)Math.Max(0.0, now - _lastT);
            _lastT = now;

            EuclidNative.Euclid_Update(_euclid, dt);
            EuclidNative.Euclid_Render(_euclid);

            if (_pendingPick)
            {
                _pendingPick = false;

                var draggingGizmoNow = EuclidNative.Euclid_IsDraggingGizmo(_euclid) != 0;
                if (!draggingGizmoNow)
                {
                    var id = EuclidNative.Euclid_RayPick(_euclid, _pendingPickX, _pendingPickY);
                    if (id != 0) 
                    {
                        EuclidNative.Euclid_SetSelection(_euclid, id);
                        Avalonia.Threading.Dispatcher.UIThread.Post(() =>
                        {
                            if (id != _lastSelection)
                            {
                                _lastSelection = id;
                                _lastTfValid = false;
                                SelectionChanged?.Invoke(id);
                            }
                        });
                    }
                }
            }

            if (EuclidNative.Euclid_GetSelection(_euclid, out var sel) == EuclidResult.EUCLID_OK)
            {
                if (sel != _lastSelection)
                {
                    _lastSelection = sel;
                    _lastTfValid = false; 
                    SelectionChanged?.Invoke(sel);
                }
            }

            bool draggingGizmo = EuclidNative.Euclid_IsDraggingGizmo(_euclid) != 0;

            if (_lastSelection != 0 &&
                EuclidNative.Euclid_GetObjectTransform(_euclid, _lastSelection, out var tf) == EuclidResult.EUCLID_OK)
            {
                if (draggingGizmo)
                {
                    _lastTf = tf;
                    _lastTfValid = true;
                    Avalonia.Threading.Dispatcher.UIThread.Post(() => TransformPolled?.Invoke(_lastSelection, tf));
                }
                else
                {
                    const float Eps = 1e-5f;
                    bool changed =
                        !_lastTfValid ||
                        Math.Abs(tf.posX - _lastTf.posX) > Eps ||
                        Math.Abs(tf.posY - _lastTf.posY) > Eps ||
                        Math.Abs(tf.posZ - _lastTf.posZ) > Eps ||
                        Math.Abs(tf.rotX - _lastTf.rotX) > Eps ||
                        Math.Abs(tf.rotY - _lastTf.rotY) > Eps ||
                        Math.Abs(tf.rotZ - _lastTf.rotZ) > Eps ||
                        Math.Abs(tf.sclX - _lastTf.sclX) > Eps ||
                        Math.Abs(tf.sclY - _lastTf.sclY) > Eps ||
                        Math.Abs(tf.sclZ - _lastTf.sclZ) > Eps;

                    if (changed)
                    {
                        _lastTf = tf;
                        _lastTfValid = true;
                        Avalonia.Threading.Dispatcher.UIThread.Post(() => TransformPolled?.Invoke(_lastSelection, tf));
                    }
                }
            }
            else
            {
                _lastTfValid = false;
            }

            RequestNextFrameRendering();
        }

        private void SendResizeIfNeeded()
        {
            if (_euclid == IntPtr.Zero) return;
            var (w, h) = GetPixelSize();
            if (w != _sentW || h != _sentH)
            {
                EuclidNative.Euclid_Resize(_euclid, w, h);
                _sentW = w;
                _sentH = h;
            }
        }

        // ===== Public API =====

        public ulong GetSelection() => _lastSelection;

        public void SetSelection(ulong id)
        {
            if (!IsReady) return;
            EuclidNative.Euclid_SetSelection(_euclid, id);
        }

        public Task ClearSceneAsync()
        {
            var tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);

            EnqueueGlJob(() =>
            {
                if (_euclid != IntPtr.Zero)
                {
                    EuclidNative.Euclid_ClearScene(_euclid);
                    EuclidNative.Euclid_SetSelection(_euclid, 0);
                    _lastSelection = 0;
                    _lastTfValid = false;
                }
                tcs.TrySetResult();
            });

            return tcs.Task;
        }

        public Task<bool> DeleteSelectedAsync()
        {
            var tcs = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);

            EnqueueGlJob(() =>
            {
                if (_euclid == IntPtr.Zero) { tcs.TrySetResult(false); return; }

                ulong sel = 0;
                if (EuclidNative.Euclid_GetSelection(_euclid, out var id) == EuclidResult.EUCLID_OK)
                    sel = id;

                var ok = sel != 0 && EuclidNative.Euclid_DeleteObject(_euclid, sel) == EuclidResult.EUCLID_OK;
                if (ok)
                {
                    EuclidNative.Euclid_SetSelection(_euclid, 0);
                    _lastSelection = 0;
                    _lastTfValid = false;
                }
                tcs.TrySetResult(ok);
            });

            return tcs.Task;
        }


        public ulong CreateCube(float edge = 1.0f, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_CUBE, new EuclidCubeParams { size = edge }, tf);

        public ulong CreateSphere(float radius = 0.5f, int slices = 24, int stacks = 24, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_SPHERE,
                new EuclidSphereParams { radius = radius, slices = slices, stacks = stacks }, tf);

        public ulong CreateTorus(float major = 0.7f, float minor = 0.25f, int segU = 32, int segV = 16, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_TORUS,
                new EuclidTorusParams { majorRadius = major, minorRadius = minor, majorSeg = segU, minorSeg = segV }, tf);

        public ulong CreatePlane(float w = 1.0f, float h = 1.0f, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_PLANE, new EuclidPlaneParams { width = w, height = h }, tf);

        public ulong CreateCone(float r = 0.5f, float hei = 1.0f, int seg = 32, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_CONE, new EuclidConeParams { radius = r, height = hei, segments = seg }, tf);

        public ulong CreateCylinder(float r = 0.5f, float hei = 1.0f, int seg = 32, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_CYLINDER, new EuclidCylinderParams { radius = r, height = hei, segments = seg }, tf);

        public ulong CreatePrism(int sides = 6, float r = 0.5f, float hei = 1.0f, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_PRISM, new EuclidPrismParams { sides = sides, radius = r, height = hei }, tf);

        public ulong CreateCircle(float r = 0.5f, int seg = 64, EuclidTransform? tf = null)
            => CreateWithParams(EuclidShapeType.EUCLID_SHAPE_CIRCLE, new EuclidCircleParams { radius = r, segments = seg }, tf);

        private static EuclidTransform DefaultTF() => new EuclidTransform
        {
            posX = 0,
            posY = 0,
            posZ = 0,
            rotX = 0,
            rotY = 0,
            rotZ = 0,
            sclX = 1,
            sclY = 1,
            sclZ = 1
        };

        private ulong CreateWithParams<TParam>(EuclidShapeType type, in TParam param, EuclidTransform? tf) where TParam : struct
        {
            if (!IsReady) return 0;

            var desc = new EuclidCreateShapeDesc
            {
                type = type,
                @params = IntPtr.Zero,
                xform = tf ?? DefaultTF()
            };

            try
            {
                desc.@params = EuclidNative.AllocParamBlob(param);
                if (EuclidNative.Euclid_CreateShape(_euclid, ref desc, out var id) == EuclidResult.EUCLID_OK && id != 0)
                {
                    EuclidNative.Euclid_SetSelection(_euclid, id);
                    _lastSelection = id;
                    _lastTfValid = false;
                    return id;
                }
            }
            finally
            {
                var p = desc.@params;
                EuclidNative.FreeParamBlob(ref p);
            }
            return 0;
        }


        public Task<ulong> ImportObjAsync(string path, bool normalize)
        {
            if (!IsReady || string.IsNullOrWhiteSpace(path))
                return Task.FromResult(0UL);

            var tcs = new TaskCompletionSource<ulong>(TaskCreationOptions.RunContinuationsAsynchronously);

            EnqueueGlJob(() =>
            {
                if (_euclid == IntPtr.Zero)
                {
                    tcs.TrySetResult(0);
                    return;
                }

                var ok = EuclidNative.Euclid_LoadOBJ(_euclid, path, out var newId, normalize ? 1 : 0) == EuclidResult.EUCLID_OK;

                if (ok && newId != 0)
                {
                    EuclidNative.Euclid_SetSelection(_euclid, newId);
                    _lastSelection = newId;
                    _lastTfValid = false;
                }

                tcs.TrySetResult(ok ? newId : 0UL);
            });

            return tcs.Task;
        }


        public void SetGizmo(EuclidGizmoMode mode)
        {
            if (!IsReady) return;
            EuclidNative.Euclid_SetGizmoMode(_euclid, mode);
        }

        public EuclidGizmoMode GetGizmo() =>
            IsReady ? EuclidNative.Euclid_GetGizmoMode(_euclid) : EuclidGizmoMode.EUCLID_GIZMO_NONE;

        public bool TryGetTransform(ulong id, out EuclidTransform tf)
        {
            tf = default;
            if (!IsReady || id == 0) return false;
            return EuclidNative.Euclid_GetObjectTransform(_euclid, id, out tf) == EuclidResult.EUCLID_OK;
        }

        public bool TrySetTransform(ulong id, in EuclidTransform tf)
        {
            if (!IsReady || id == 0) return false;
            var t = tf;
            return EuclidNative.Euclid_SetObjectTransform(_euclid, id, ref t) == EuclidResult.EUCLID_OK;
        }

        // ===== Input from host =====

        public void UpdateMods(KeyModifiers km)
        {
            if (_euclid == IntPtr.Zero) return;
            EuclidNative.Euclid_OnMods(_euclid, EuclidNative.ToEuclidMods(km));
        }

        public void HostPointerPressed(PointerPressedEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            Focus();
            UpdateMods(e.KeyModifiers);

            var s = GetScale();
            var ptL = e.GetPosition(this);
            var px = ptL.X * s;
            var py = ptL.Y * s;

            _lastPt = ptL;
            _virtX = px;
            _virtY = py;

            var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;
            var mods = EuclidNative.ToEuclidMods(e.KeyModifiers);

            if (kind == PointerUpdateKind.MiddleButtonPressed)
            {
                _dragging = true;
                _orbitViaCtrlLeft = false;

                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseMove(_euclid, px, py);
                    EuclidNative.Euclid_OnMouseButton(_euclid, EuclidMouseButton.EUCLID_MOUSE_MIDDLE, 1, mods);
                });

                e.Handled = true;
                return;
            }

            if (kind == PointerUpdateKind.LeftButtonPressed && (e.KeyModifiers & KeyModifiers.Control) != 0)
            {
                _dragging = true;
                _orbitViaCtrlLeft = true;

                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseMove(_euclid, px, py);
                    EuclidNative.Euclid_OnMouseButton(_euclid, EuclidMouseButton.EUCLID_MOUSE_LEFT, 1, mods);
                });

                e.Handled = true;
                return;
            }

            if (TryGetButton(kind, out var btn, out var isDown) && isDown)
            {
                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseButton(_euclid, btn, 1, mods);
                });
                e.Handled = true;
                return;
            }

            e.Handled = true;
        }

        public void HostPointerReleased(PointerReleasedEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            UpdateMods(e.KeyModifiers);

            var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;
            var mods = EuclidNative.ToEuclidMods(e.KeyModifiers);

            if (kind == PointerUpdateKind.LeftButtonReleased)
            {
                var s = GetScale();
                var ptL = e.GetPosition(this);
                var px = ptL.X * s;
                var py = ptL.Y * s;

                _lastPt = ptL;
                _virtX = px;
                _virtY = py;

                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseMove(_euclid, px, py);
                    EuclidNative.Euclid_OnMouseButton(_euclid, EuclidMouseButton.EUCLID_MOUSE_LEFT, 0, mods);
                });

                _pendingPickX = (float)px;
                _pendingPickY = (float)py;
                _pendingPick = true;

                _dragging = false;
                _orbitViaCtrlLeft = false;
                e.Handled = true;
                return;
            }

            UpdateMods(e.KeyModifiers);


            if (kind == PointerUpdateKind.MiddleButtonReleased)
            {
                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseButton(_euclid, EuclidMouseButton.EUCLID_MOUSE_MIDDLE, 0, mods);
                });

                _dragging = false;
                _orbitViaCtrlLeft = false;
                e.Handled = true;
                return;
            }

            if (TryGetButton(kind, out var btn2, out var isDown2) && !isDown2)
            {
                EnqueueGlJob(() =>
                {
                    if (_euclid == IntPtr.Zero) return;
                    EuclidNative.Euclid_OnMods(_euclid, mods);
                    EuclidNative.Euclid_OnMouseButton(_euclid, btn2, 0, mods);
                });
                e.Handled = true;
                return;
            }

            e.Handled = true;
        }

        public void HostPointerMoved(PointerEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            UpdateMods(e.KeyModifiers);

            var s = GetScale();
            var ptL = e.GetPosition(this);
            var px = ptL.X * s;
            var py = ptL.Y * s;

            _lastPt = ptL;
            _virtX = px;
            _virtY = py;

            var mods = EuclidNative.ToEuclidMods(e.KeyModifiers);
            EnqueueGlJob(() =>
            {
                if (_euclid == IntPtr.Zero) return;
                EuclidNative.Euclid_OnMods(_euclid, mods);
                EuclidNative.Euclid_OnMouseMove(_euclid, px, py);
            });

            e.Handled = true;
        }

        public void HostPointerWheel(PointerWheelEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            UpdateMods(e.KeyModifiers);
            EuclidNative.Euclid_OnScroll(_euclid, -e.Delta.X, -e.Delta.Y);
            e.Handled = true;
        }

        private static bool TryGetButton(PointerUpdateKind kind, out EuclidMouseButton btn, out bool isDown)
        {
            btn = EuclidMouseButton.EUCLID_MOUSE_LEFT; isDown = false;
            switch (kind)
            {
                case PointerUpdateKind.LeftButtonPressed: btn = EuclidMouseButton.EUCLID_MOUSE_LEFT; isDown = true; return true;
                case PointerUpdateKind.LeftButtonReleased: btn = EuclidMouseButton.EUCLID_MOUSE_LEFT; isDown = false; return true;
                case PointerUpdateKind.RightButtonPressed: btn = EuclidMouseButton.EUCLID_MOUSE_RIGHT; isDown = true; return true;
                case PointerUpdateKind.RightButtonReleased: btn = EuclidMouseButton.EUCLID_MOUSE_RIGHT; isDown = false; return true;
                case PointerUpdateKind.MiddleButtonPressed: btn = EuclidMouseButton.EUCLID_MOUSE_MIDDLE; isDown = true; return true;
                case PointerUpdateKind.MiddleButtonReleased: btn = EuclidMouseButton.EUCLID_MOUSE_MIDDLE; isDown = false; return true;
                default: return false;
            }
        }
    }
}
