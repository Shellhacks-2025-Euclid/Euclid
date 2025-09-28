using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using EuclidApp.ViewModels;

namespace EuclidApp.Views;

public partial class RenderImageWindow : Window
{
    public RenderImageWindow()
    {
        InitializeComponent();
        DataContext = new RenderImageViewModel(this);
    }
}