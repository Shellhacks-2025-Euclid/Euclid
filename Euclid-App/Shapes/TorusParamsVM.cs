using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class TorusParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double majorRadius = 0.7;
        [ObservableProperty] private double minorRadius = 0.25;
        [ObservableProperty] private int segU = 32;
        [ObservableProperty] private int segV = 16;
    }
}
