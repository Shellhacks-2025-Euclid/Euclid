using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class SphereParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double radius = 0.5;
        [ObservableProperty] private int slices = 24;
        [ObservableProperty] private int stacks = 24;
    }
}
