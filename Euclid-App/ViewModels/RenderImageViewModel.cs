using Avalonia.Controls;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EuclidApp.ViewModels
{
    public partial class RenderImageViewModel : ViewModelBase
    {
        private readonly Window _owner;

        [ObservableProperty]
        private string statusBarText =
            "TODO";

        public ObservableCollection<string> ZoomPresets { get; } =
            new() { "12.5%", "25%", "50%", "100%", "200%", "Fit" };

        [ObservableProperty] private string? selectedZoomPreset = "100%";

        public RenderImageViewModel(Window owner)
        {
            _owner = owner;
        }

        [RelayCommand] private void Fit() => SelectedZoomPreset = "Fit";
        [RelayCommand] private void OneToOne() => SelectedZoomPreset = "100%";

        [RelayCommand] private void Save() { /* TODO: Save boofer */ }
        [RelayCommand]
        private async Task SaveAsAsync()
        {
            var sfd = new SaveFileDialog
            {
                Title = "Save Render",
                Filters =
                {
                    new FileDialogFilter{ Name="PNG", Extensions=new(){"png"} },
                    new FileDialogFilter{ Name="JPEG", Extensions=new(){"jpg","jpeg"} },
                    new FileDialogFilter{ Name="EXR", Extensions=new(){"exr"} }
                }
            };
            await sfd.ShowAsync(_owner);
        }
        [RelayCommand] private void Copy() { /* TODO: copy from buffer */ }

        [RelayCommand] private void Close() => _owner.Close();

    }
}
