using System;
using System.Runtime.InteropServices;
using Avalonia.Input;

namespace EuclidApp.Interop
{
    // === C ABI types ===

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidConfig
    {
        public int width;
        public int height;
        public int gl_major;
        public int gl_minor;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidTransform
    {
        public float posX, posY, posZ;
        public float rotX, rotY, rotZ; // degrees
        public float sclX, sclY, sclZ;
    }

    public enum EuclidResult : int
    {
        EUCLID_OK = 0,
        EUCLID_ERR_INIT = -1,
        EUCLID_ERR_BAD_PARAM = -2,
        EUCLID_ERR_GLAD = -3
    }

    public enum EuclidMouseButton : int
    {
        EUCLID_MOUSE_LEFT = 0,
        EUCLID_MOUSE_RIGHT = 1,
        EUCLID_MOUSE_MIDDLE = 2
    }

    [Flags]
    public enum EuclidMods : byte
    {
        NONE = 0,
        SHIFT = 1 << 0,
        CTRL = 1 << 1,
        ALT = 1 << 2,
        SUPER = 1 << 3
    }

    public enum EuclidShapeType : int
    {
        EUCLID_SHAPE_CUBE = 0,
        EUCLID_SHAPE_SPHERE = 1,
        EUCLID_SHAPE_TORUS = 2,
        EUCLID_SHAPE_PLANE = 3,
        EUCLID_SHAPE_CONE = 4,
        EUCLID_SHAPE_CYLINDER = 5,
        EUCLID_SHAPE_PRISM = 6,
        EUCLID_SHAPE_CIRCLE = 7,
        EUCLID_SHAPE_CUSTOM = 8
    }

    public enum EuclidGizmoMode : int
    {
        EUCLID_GIZMO_NONE = 0,
        EUCLID_GIZMO_TRANSLATE = 1,
        EUCLID_GIZMO_ROTATE = 2,
        EUCLID_GIZMO_SCALE = 3
    }

    // per-shape params
    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidCubeParams { public float size; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidSphereParams { public float radius; public int slices; public int stacks; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidTorusParams
    {
        public float majorRadius; public float minorRadius;
        public int majorSeg; public int minorSeg;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidPlaneParams { public float width; public float height; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidConeParams { public float radius; public float height; public int segments; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidCylinderParams { public float radius; public float height; public int segments; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidPrismParams { public int sides; public float radius; public float height; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidCircleParams { public float radius; public int segments; }

    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidCreateShapeDesc
    {
        public EuclidShapeType type;
        public IntPtr @params; // points to native copy of one of the *_Params above
        public EuclidTransform xform;
    }

    // loader: const char* -> IntPtr, CC = Cdecl
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate IntPtr Euclid_GetProcAddr([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    public static class EuclidNative
    {
        const string Dll = "Euclid-Lib";

        // --- lifecycle / rendering
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Euclid_Create(ref EuclidConfig cfg, Euclid_GetProcAddr loader, out IntPtr outHandle);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_Destroy(IntPtr h);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_Resize(IntPtr h, int w, int hgt);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_Update(IntPtr h, float dt);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_Render(IntPtr h);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_SetFramebuffer(IntPtr h, uint fb);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Euclid_Version();

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Euclid_GetLastError();

        public static string PtrToUtf8Str(IntPtr p)
            => p == IntPtr.Zero ? string.Empty : Marshal.PtrToStringUTF8(p) ?? string.Empty;

        // --- input
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnMouseMove(IntPtr h, double x, double y);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnMouseButton(IntPtr h, EuclidMouseButton b, int down, byte mods);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnScroll(IntPtr h, double dx, double dy);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnMods(IntPtr h, byte mods);

        // --- scene / objects
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_ClearScene(IntPtr h);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_CreateShape(IntPtr h, ref EuclidCreateShapeDesc desc, out ulong outId);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_DeleteObject(IntPtr h, ulong id);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_GetSelection(IntPtr h, out ulong outId);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_SetSelection(IntPtr h, ulong id);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Euclid_IsDraggingGizmo(IntPtr h); // 0/1

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_SetGizmoMode(IntPtr h, EuclidGizmoMode mode);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidGizmoMode Euclid_GetGizmoMode(IntPtr h);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_GetObjectTransform(IntPtr h, ulong id, out EuclidTransform tf);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_SetObjectTransform(IntPtr h, ulong id, ref EuclidTransform tf);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong Euclid_RayPick(IntPtr h, float x, float y);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_HitTestSelect(IntPtr h, double x, double y, out ulong outId);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern EuclidResult Euclid_LoadOBJ(
            IntPtr h,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
            out ulong outId,
            int normalize);

       

        // --- helpers ---

        public static byte ToEuclidMods(KeyModifiers km)
        {
            EuclidMods m = EuclidMods.NONE;
            if ((km & KeyModifiers.Shift) != 0) m |= EuclidMods.SHIFT;
            if ((km & KeyModifiers.Control) != 0) m |= EuclidMods.CTRL;
            if ((km & KeyModifiers.Alt) != 0) m |= EuclidMods.ALT;
            if ((km & KeyModifiers.Meta) != 0) m |= EuclidMods.SUPER;
            return (byte)m;
        }

        public static IntPtr AllocParamBlob<T>(in T value) where T : struct
        {
            var size = Marshal.SizeOf<T>();
            var p = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(value, p, fDeleteOld: false);
            return p;
        }

        public static void FreeParamBlob(ref IntPtr p)
        {
            if (p != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(p);
                p = IntPtr.Zero;
            }
        }
    }
}
