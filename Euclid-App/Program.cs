// Program.cs
using Avalonia;
using Avalonia.OpenGL;
using Avalonia.ReactiveUI;
using Avalonia.Win32;
using EuclidApp;

public static class Program
{
    public static AppBuilder BuildAvaloniaApp() =>
        AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .With(new Win32PlatformOptions
            {
                RenderingMode = new[] { Win32RenderingMode.Wgl, Win32RenderingMode.Software },

                WglProfiles = new[]
                {
                    new GlVersion(GlProfileType.OpenGL, 3, 3),
                }
            })
            .UseReactiveUI()
            .LogToTrace();

    public static void Main(string[] args) =>
        BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
}
