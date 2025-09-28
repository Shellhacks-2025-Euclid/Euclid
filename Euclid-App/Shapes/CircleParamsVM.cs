using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class CircleParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double radius = 0.5;
        [ObservableProperty] private int segments = 64;
    }
}
