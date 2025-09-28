using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using EuclidApp.Controls;
using EuclidApp.ViewModels;

namespace EuclidApp;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainViewModel();

        if (DataContext is MainViewModel vm)
        {
            var tVm = Trellis.VM;
            tVm.Submitted += (prompt, images) =>
            {
                vm.HandleTrellisSubmit(prompt, images);
            };

            Trellis.Closed += () => vm.IsPromptOpen = false;

            vm.PropertyChanged += (_, e) =>
            {
                if (e.PropertyName == nameof(MainViewModel.IsPromptOpen))
                {
                    if (vm.IsPromptOpen) Trellis.Open();
                    else Trellis.Close();
                }
            };
        }
    }
}
