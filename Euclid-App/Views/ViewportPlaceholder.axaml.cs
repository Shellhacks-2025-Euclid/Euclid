// EuclidApp/Views/ViewportPlaceholder.axaml.cs
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;

namespace EuclidApp.Views;

public partial class ViewportPlaceholder : UserControl
{
    public ViewportPlaceholder()
    {
        InitializeComponent();

        InputLayer.PointerPressed += (_, e) => { Capture(InputLayer, e); GL.HostPointerPressed(e); };
        InputLayer.PointerReleased += (_, e) => { Release(InputLayer, e); GL.HostPointerReleased(e); };
        InputLayer.PointerMoved += (_, e) => { GL.HostPointerMoved(e); };
        InputLayer.PointerWheelChanged += (_, e) => { GL.HostPointerWheel(e); };

        InputLayer.AddHandler(KeyDownEvent, OnKeyChanged, handledEventsToo: true);
        InputLayer.AddHandler(KeyUpEvent, OnKeyChanged, handledEventsToo: true);

        InputLayer.AttachedToVisualTree += (_, __) => InputLayer.Focus();
    }

    private void OnKeyChanged(object? sender, KeyEventArgs e)
    {
        GL.UpdateMods(e.KeyModifiers);
    }

    private static void Capture(IInputElement el, PointerPressedEventArgs e)
    {
        e.Pointer.Capture(el);
    }

    private static void Release(IInputElement el, PointerReleasedEventArgs e)
    {
        if (e.Pointer.Captured == el)
            e.Pointer.Capture(null);
    }
}
