using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public abstract class ShapeParamsVM : ObservableObject { }

    public sealed class CustomParamsVM : ShapeParamsVM { }
}
