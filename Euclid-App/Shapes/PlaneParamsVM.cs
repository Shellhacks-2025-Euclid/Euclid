using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models.Shapes
{
    public sealed partial class PlaneParamsVM : ShapeParamsVM
    {
        [ObservableProperty] private double width = 1.0;
        [ObservableProperty] private double height = 1.0;
    }
}
