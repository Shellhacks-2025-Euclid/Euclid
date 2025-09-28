using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class PrismParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private int sides = 6;
        [ObservableProperty] private double radius = 0.5;
        [ObservableProperty] private double height = 1.0;
    }
}
