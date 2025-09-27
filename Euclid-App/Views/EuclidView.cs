// EuclidApp/Views/EuclidView.cs
using System;
using System.Diagnostics;
using Avalonia;
using Avalonia.Input;
using Avalonia.OpenGL;
using Avalonia.OpenGL.Controls;
using EuclidApp.Interop;

namespace EuclidApp.Views
{
    public sealed class EuclidView : OpenGlControlBase
    {
        private IntPtr _euclid = IntPtr.Zero;
        private readonly Stopwatch _timer = new();
        private double _lastT;
        private int _sentW, _sentH;

        private bool _dragging;             
        private bool _orbitViaCtrlLeft;     
        private Point _lastPt;
        private double _virtX, _virtY;
        private const double DragSensitivity = 1.0;

        public EuclidView()
        {
            Focusable = true;
            IsHitTestVisible = true;
            SizeChanged += (_, __) => SendResizeIfNeeded();
        }

        public void UpdateMods(KeyModifiers km)
        {
            if (_euclid == IntPtr.Zero) return;
            EuclidNative.Euclid_OnMods(_euclid, EuclidNative.ToEuclidMods(km));
        }

        // ---------- OpenGL ----------
        protected override void OnOpenGlInit(GlInterface gl)
        {
            var w = Math.Max(1, (int)Bounds.Width);
            var h = Math.Max(1, (int)Bounds.Height);
            var cfg = new EuclidConfig { width = w, height = h, gl_major = 3, gl_minor = 3 };

            if (EuclidNative.Euclid_Create(ref cfg, gl.GetProcAddress, out _euclid) != 0 || _euclid == IntPtr.Zero)
                return;

            _sentW = _sentH = 0;
            SendResizeIfNeeded();

            _timer.Restart();
            _lastT = 0;
            RequestNextFrameRendering();
        }

        protected override void OnOpenGlDeinit(GlInterface gl)
        {
            if (_euclid != IntPtr.Zero)
            {
                EuclidNative.Euclid_Destroy(_euclid);
                _euclid = IntPtr.Zero;
            }
        }

        protected override void OnOpenGlRender(GlInterface gl, int fb)
        {
            if (_euclid == IntPtr.Zero) return;

            EuclidNative.Euclid_SetFramebuffer(_euclid, (uint)fb);
            SendResizeIfNeeded();

            var now = _timer.Elapsed.TotalSeconds;
            var dt = (float)Math.Max(0.0, now - _lastT);
            _lastT = now;

            EuclidNative.Euclid_Update(_euclid, dt);
            EuclidNative.Euclid_Render(_euclid);

            RequestNextFrameRendering();
        }

        private void SendResizeIfNeeded()
        {
            if (_euclid == IntPtr.Zero) return;
            var w = Math.Max(1, (int)Bounds.Width);
            var h = Math.Max(1, (int)Bounds.Height);
            if (w != _sentW || h != _sentH)
            {
                EuclidNative.Euclid_Resize(_euclid, w, h);
                _sentW = w; _sentH = h;
            }
        }

        public void HostPointerPressed(PointerPressedEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            Focus();
            UpdateMods(e.KeyModifiers); 

            var pt = e.GetPosition(this);
            _lastPt = pt;

            var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;

            // 1) «Чистый Blender»: MMB — жмём, шлём в DLL
            if (TryGetButton(kind, out var btn, out var isDown) && isDown)
            {
                EuclidNative.Euclid_OnMouseButton(_euclid, btn, 1, EuclidNative.ToEuclidMods(e.KeyModifiers));
                if (kind == PointerUpdateKind.MiddleButtonPressed)
                {
                    _dragging = true;
                    _orbitViaCtrlLeft = false;
                    _virtX = pt.X; _virtY = pt.Y;
                    EuclidNative.Euclid_OnMouseMove(_euclid, _virtX, _virtY);
                }
            }

            if ((e.KeyModifiers & KeyModifiers.Control) != 0 && kind == PointerUpdateKind.LeftButtonPressed)
            {
                _dragging = true;
                _orbitViaCtrlLeft = true; 
                _virtX = pt.X; _virtY = pt.Y;
                EuclidNative.Euclid_OnMouseMove(_euclid, _virtX, _virtY);
            }

            e.Handled = true;
        }

        public void HostPointerReleased(PointerReleasedEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            UpdateMods(e.KeyModifiers);

            var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;
            if (TryGetButton(kind, out var btn, out var isDown))
            {
                if (!isDown)
                    EuclidNative.Euclid_OnMouseButton(_euclid, btn, 0, EuclidNative.ToEuclidMods(e.KeyModifiers));

                if (kind == PointerUpdateKind.MiddleButtonReleased)
                {
                    _dragging = false;
                    _orbitViaCtrlLeft = false;
                }
            }

            if (_orbitViaCtrlLeft && kind == PointerUpdateKind.LeftButtonReleased)
            {
                _dragging = false;
                _orbitViaCtrlLeft = false;
            }

            e.Handled = true;
        }

        public void HostPointerMoved(PointerEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;

            UpdateMods(e.KeyModifiers);

            var pt = e.GetPosition(this);
            if (_dragging)
            {
                var dx = (pt.X - _lastPt.X) * DragSensitivity;
                var dy = (pt.Y - _lastPt.Y) * DragSensitivity;

                _virtX = Math.Clamp(_virtX + dx, 0, Math.Max(1, (int)Bounds.Width));
                _virtY = Math.Clamp(_virtY + dy, 0, Math.Max(1, (int)Bounds.Height));

                EuclidNative.Euclid_OnMouseMove(_euclid, _virtX, _virtY);
            }
            else
            {
                EuclidNative.Euclid_OnMouseMove(_euclid, pt.X, pt.Y);
            }

            _lastPt = pt;
            e.Handled = true;
        }

        public void HostPointerWheel(PointerWheelEventArgs e)
        {
            if (_euclid == IntPtr.Zero) return;
            UpdateMods(e.KeyModifiers);
            EuclidNative.Euclid_OnScroll(_euclid, e.Delta.X, e.Delta.Y);
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
