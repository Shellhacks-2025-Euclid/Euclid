using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media.Imaging;
using Avalonia.Platform.Storage;
using Avalonia.Threading;
using EuclidApp.ViewModels;
using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace EuclidApp.Controls
{
    public partial class TrellisPrompt : UserControl
    {
        public TrellisPromptViewModel VM => (TrellisPromptViewModel)DataContext!;

        // —обытие Ч контрол закрылс€ сам (фон/ESC)
        public event Action? Closed;

        private const int AnimMs = 160;

        public TrellisPrompt()
        {
            InitializeComponent();
            DataContext = new TrellisPromptViewModel();

            AttachedToVisualTree += (_, __) =>
            {
                VM.PanelOpacity = 0.0;
                VM.SlideY = 18;

                if (Backdrop != null)
                    Backdrop.AddHandler(InputElement.PointerPressedEvent, OnBackdropPressed, RoutingStrategies.Tunnel);

                if (DropZone != null)
                {
                    DragDrop.SetAllowDrop(DropZone, true);
                    DropZone.AddHandler(DragDrop.DragOverEvent, OnDragOver, RoutingStrategies.Tunnel | RoutingStrategies.Bubble);
                    DropZone.AddHandler(DragDrop.DropEvent, OnDrop, RoutingStrategies.Tunnel | RoutingStrategies.Bubble);
                }

                if (AddBtn != null) AddBtn.Click += OnAddFileClicked;
                if (PasteBtn != null) PasteBtn.Click += OnPasteImageClicked;

                if (PromptBox != null) PromptBox.KeyDown += OnPromptBoxKeyDown;

                if (PreviewItems != null)
                    PreviewItems.AddHandler(Button.ClickEvent, OnPreviewButtonClick, RoutingStrategies.Bubble);
            };
        }

        public async void Open()
        {
            VM.IsOpen = true;
            VM.PanelOpacity = 0.0;
            VM.SlideY = 18;

            await Dispatcher.UIThread.InvokeAsync(() => PromptBox?.Focus());

            VM.PanelOpacity = 1.0;
            VM.SlideY = 0;
        }

        public async void Close()
        {
            VM.SlideY = 18;
            VM.PanelOpacity = 0.0;

            await Task.Delay(AnimMs);
            VM.IsOpen = false;

            Closed?.Invoke();
        }

        private void OnBackdropPressed(object? sender, PointerPressedEventArgs e)
        {
            e.Handled = true;
            Close();
        }

        // √ор€чие клавиши
        private void OnPromptBoxKeyDown(object? sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                e.Handled = true;
                Close();
                return;
            }

            if (e.Key == Key.Enter && (e.KeyModifiers & KeyModifiers.Shift) == 0)
            {
                if (VM.SendCommand.CanExecute(null))
                    VM.SendCommand.Execute(null);
                e.Handled = true;
                return;
            }

            if (e.Key == Key.V && (e.KeyModifiers & KeyModifiers.Control) != 0)
            {
                _ = TryPasteImageAsync();
            }
        }

        private async void OnPasteImageClicked(object? sender, RoutedEventArgs e)
        {
            await TryPasteImageAsync();
        }

        private async Task TryPasteImageAsync()
        {
            var top = TopLevel.GetTopLevel(this);
            if (top?.Clipboard == null) return;

            var formats = await top.Clipboard.GetFormatsAsync();
            if (formats is { Length: > 0 })
            {
                if (formats.Contains(DataFormats.FileNames))
                {
                    if (await top.Clipboard.GetDataAsync(DataFormats.FileNames) is string[] names && names.Length > 0)
                    {
                        foreach (var path in names)
                            await TryAddImageFromPath(path);
                        return;
                    }
                }

                if (formats.Contains(DataFormats.Files))
                {
                    if (await top.Clipboard.GetDataAsync(DataFormats.Files) is IEnumerable<IStorageItem> items)
                    {
                        await AddFilesAsync(items);
                        return;
                    }
                }

                var imgFormat = formats.FirstOrDefault(f =>
                    f.Contains("image/", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("png", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("jpeg", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("jpg", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("bmp", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("gif", StringComparison.OrdinalIgnoreCase) ||
                    f.Contains("webp", StringComparison.OrdinalIgnoreCase));

                if (imgFormat != null)
                {
                    var data = await top.Clipboard.GetDataAsync(imgFormat);
                    if (await TryDecodeClipboardImageAsync(data, imgFormat)) return;
                }

                var octet = formats.FirstOrDefault(f => f.Contains("octet", StringComparison.OrdinalIgnoreCase));
                if (octet != null)
                {
                    var data = await top.Clipboard.GetDataAsync(octet);
                    if (await TryDecodeClipboardImageAsync(data, octet)) return;
                }
            }

            var text = await top.Clipboard.GetTextAsync();
            if (!string.IsNullOrWhiteSpace(text))
            {
                text = text.Trim();

                if (text.StartsWith("data:image/", StringComparison.OrdinalIgnoreCase))
                {
                    var i = text.IndexOf(',');
                    if (i > 0)
                    {
                        try
                        {
                            var bytes = Convert.FromBase64String(text[(i + 1)..]);
                            using var ms = new MemoryStream(bytes);
                            VM.AddBitmap(new Bitmap(ms), "clipboard.png");
                            return;
                        }
                        catch { /* ignore */ }
                    }
                }

                if (Uri.TryCreate(text, UriKind.Absolute, out var uri) && uri.IsFile)
                {
                    await TryAddImageFromPath(uri.LocalPath);
                    return;
                }
                if (File.Exists(text))
                {
                    await TryAddImageFromPath(text);
                    return;
                }
            }
        }

        private async Task<bool> TryDecodeClipboardImageAsync(object? data, string? formatHint)
        {
            try
            {
                switch (data)
                {
                    case byte[] bytes:
                        using (var ms = new MemoryStream(bytes))
                        {
                            VM.AddBitmap(new Bitmap(ms), GuessName(formatHint));
                            return true;
                        }

                    case Stream stream:
                        using (var ms = new MemoryStream())
                        {
                            await stream.CopyToAsync(ms);
                            ms.Position = 0;
                            VM.AddBitmap(new Bitmap(ms), GuessName(formatHint));
                            return true;
                        }

                    case string s when s.StartsWith("data:image/", StringComparison.OrdinalIgnoreCase):
                        {
                            var i = s.IndexOf(',');
                            if (i > 0)
                            {
                                var bytes2 = Convert.FromBase64String(s[(i + 1)..]);
                                using var ms2 = new MemoryStream(bytes2);
                                VM.AddBitmap(new Bitmap(ms2), GuessName(formatHint));
                                return true;
                            }
                            break;
                        }
                }
            }
            catch
            {

            }
            return false;

            static string GuessName(string? hint)
            {
                if (string.IsNullOrEmpty(hint)) return "clipboard.png";
                if (hint.Contains("png", StringComparison.OrdinalIgnoreCase)) return "clipboard.png";
                if (hint.Contains("jpg", StringComparison.OrdinalIgnoreCase) ||
                    hint.Contains("jpeg", StringComparison.OrdinalIgnoreCase)) return "clipboard.jpg";
                if (hint.Contains("webp", StringComparison.OrdinalIgnoreCase)) return "clipboard.webp";
                if (hint.Contains("gif", StringComparison.OrdinalIgnoreCase)) return "clipboard.gif";
                if (hint.Contains("bmp", StringComparison.OrdinalIgnoreCase)) return "clipboard.bmp";
                return "clipboard.png";
            }
        }

        private async Task TryAddImageFromPath(string path)
        {
            try
            {
                await using var s = File.OpenRead(path);
                VM.AddBitmap(new Bitmap(s), System.IO.Path.GetFileName(path));
            }
            catch { /* ignore */ }
        }

        private async void OnAddFileClicked(object? sender, RoutedEventArgs e)
        {
            var top = TopLevel.GetTopLevel(this);
            if (top?.StorageProvider == null) return;

            var files = await top.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
            {
                Title = "Add images",
                AllowMultiple = true,
                FileTypeFilter = new[]
                {
                    new FilePickerFileType("Images")
                    {
                        Patterns = new[] { "*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.webp","*.tiff" }
                    }
                }
            });

            if (files != null && files.Count > 0)
                await AddFilesAsync(files);
        }

        // DragOver/Drop
        private void OnDragOver(object? sender, DragEventArgs e)
        {
            if (e.Data.Contains(DataFormats.Files) || e.Data.Contains(DataFormats.FileNames))
            {
                e.DragEffects = DragDropEffects.Copy;
                e.Handled = true;
            }
        }

        private async void OnDrop(object? sender, DragEventArgs e)
        {
            var storageFiles = e.Data.GetFiles();
            if (storageFiles != null)
            {
                await AddFilesAsync(storageFiles);
                return;
            }

            if (e.Data.Contains(DataFormats.FileNames))
            {
                var paths = e.Data.GetFileNames();
                if (paths != null)
                {
                    foreach (var path in paths)
                        await TryAddImageFromPath(path);
                }
            }
        }

        private async Task AddFilesAsync(IEnumerable<IStorageItem> items)
        {
            foreach (var f in items.OfType<IStorageFile>())
            {
                try
                {
                    await using var s = await f.OpenReadAsync();
                    VM.AddBitmap(new Bitmap(s), f.Name);
                }
                catch { /* ignore */ }
            }
        }

        private void OnPreviewButtonClick(object? sender, RoutedEventArgs e)
        {
            if (e.Source is Button btn && btn.Tag is AttachmentItem item)
            {
                VM.RemoveAttachment(item);
                e.Handled = true;
            }
        }
    }
}
