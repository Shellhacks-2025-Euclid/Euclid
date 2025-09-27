using Avalonia.Input;
using System;
using System.Runtime.InteropServices;

namespace EuclidApp.Interop
{
    [StructLayout(LayoutKind.Sequential)]
    public struct EuclidConfig
    {
        public int width;
        public int height;
        public int gl_major;
        public int gl_minor;
    }

    // loader: const char* -> IntPtr, CC = Cdecl
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate IntPtr Euclid_GetProcAddr(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    // mods = uint8_t -> byte в C#
    public enum EuclidMouseButton : int
    {
        EUCLID_MOUSE_LEFT = 0,
        EUCLID_MOUSE_RIGHT = 1,
        EUCLID_MOUSE_MIDDLE = 2
    }

    public static class EuclidNative
    {
        const string Dll = "Euclid-Lib";

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

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnMouseMove(IntPtr h, double x, double y);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnMouseButton(IntPtr h, EuclidMouseButton b, int down, byte mods);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Euclid_OnScroll(IntPtr h, double dx, double dy);

        private static byte ToEuclidMods(KeyModifiers km)
        {
            byte m = 0;
            if ((km & KeyModifiers.Shift) != 0) m |= EuclidMods.SHIFT;
            if ((km & KeyModifiers.Control) != 0) m |= EuclidMods.CTRL;
            if ((km & KeyModifiers.Alt) != 0) m |= EuclidMods.ALT;
            if ((km & KeyModifiers.Meta) != 0) m |= EuclidMods.SUPER;
            return m;
        }

    }

    public static class EuclidMods
    {
        public const byte NONE = 0;
        public const byte SHIFT = 1 << 0;
        public const byte CTRL = 1 << 1;
        public const byte ALT = 1 << 2;
        public const byte SUPER = 1 << 3;

    }
}
