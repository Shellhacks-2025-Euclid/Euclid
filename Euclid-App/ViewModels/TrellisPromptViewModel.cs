using Avalonia.Media.Imaging;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.ObjectModel;
using System.ComponentModel.Design;
using System.Linq;

namespace EuclidApp.ViewModels
{
    public sealed partial class TrellisPromptViewModel : ViewModelBase
    {
        [ObservableProperty] private bool isOpen;
        [ObservableProperty] private double panelOpacity = 1.0;
        [ObservableProperty] private double slideY = 24.0;

        [ObservableProperty] private string prompt = string.Empty;

        public ObservableCollection<AttachmentItem> Attachments { get; } = new();

        public string AttachmentInfo =>
            Attachments.Count == 0 ? "No images"
            : Attachments.Count == 1 ? "1 image"
            : $"{Attachments.Count} images";

        public bool CanSend => !string.IsNullOrWhiteSpace(Prompt) || Attachments.Any();

        public event Action<string, AttachmentItem[]?>? Submitted; 

        public TrellisPromptViewModel()
        {
        }

        [RelayCommand(CanExecute = nameof(CanSend))]
        private void Send()
        {
            var list = Attachments.ToArray();
            Submitted?.Invoke(Prompt.Trim(), list);
            Prompt = string.Empty;
            Attachments.Clear();
        }

        public void AddBitmap(Bitmap bmp, string name)
        {
            Attachments.Add(new AttachmentItem(bmp, name));
            OnPropertyChanged(nameof(AttachmentInfo));
            OnPropertyChanged(nameof(CanSend));
            SendCommand.NotifyCanExecuteChanged();
        }

        public void RemoveAttachment(AttachmentItem item)
        {
            Attachments.Remove(item);
            OnPropertyChanged(nameof(AttachmentInfo));
            OnPropertyChanged(nameof(CanSend));
            SendCommand.NotifyCanExecuteChanged();
        }

        partial void OnPromptChanged(string value)
        {
            OnPropertyChanged(nameof(CanSend));
            SendCommand.NotifyCanExecuteChanged();
        }

        partial void OnIsOpenChanged(bool value)
        {
            PanelOpacity = value ? 1.0 : 0.0;
        }
    }

    public sealed class AttachmentItem
    {
        public Bitmap Thumbnail { get; }
        public string FileName { get; }

        public AttachmentItem(Bitmap thumbnail, string fileName)
        {
            Thumbnail = thumbnail;
            FileName = fileName;
        }
    }
}
