using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class CubeParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double edge = 1.0;
    }
}
