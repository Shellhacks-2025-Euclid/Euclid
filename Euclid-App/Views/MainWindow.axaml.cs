using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using EuclidApp.ViewModels;

namespace EuclidApp;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainViewModel();
    }
}