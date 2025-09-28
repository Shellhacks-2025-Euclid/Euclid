using CommunityToolkit.Mvvm.ComponentModel;

namespace EuclidApp.Models
{
    public sealed partial class Transform : ObservableObject
    {
        [ObservableProperty] private double posX;
        [ObservableProperty] private double posY;
        [ObservableProperty] private double posZ;

        [ObservableProperty] private double rotX;
        [ObservableProperty] private double rotY;
        [ObservableProperty] private double rotZ;

        [ObservableProperty] private double sclX = 1.0;
        [ObservableProperty] private double sclY = 1.0;
        [ObservableProperty] private double sclZ = 1.0;
    }
}
