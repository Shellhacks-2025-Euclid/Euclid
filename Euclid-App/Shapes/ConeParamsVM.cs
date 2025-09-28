using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class ConeParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double radius = 0.5;
        [ObservableProperty] private double height = 1.0;
        [ObservableProperty] private int segments = 32;
    }
}
