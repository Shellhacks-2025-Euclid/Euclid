using System;
using System.Globalization;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;

namespace EuclidApp.Controls
{
    public partial class NumericField : UserControl
    {
        // Styled properties
        public static readonly StyledProperty<double?> ValueProperty =
            AvaloniaProperty.Register<NumericField, double?>(nameof(Value),
                defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

        public static readonly StyledProperty<double> StepProperty =
            AvaloniaProperty.Register<NumericField, double>(nameof(Step), 0.1);

        public static readonly StyledProperty<double?> MinProperty =
            AvaloniaProperty.Register<NumericField, double?>(nameof(Min));

        public static readonly StyledProperty<double?> MaxProperty =
            AvaloniaProperty.Register<NumericField, double?>(nameof(Max));

        public static readonly StyledProperty<bool> WrapProperty =
            AvaloniaProperty.Register<NumericField, bool>(nameof(Wrap), false);

        public static readonly StyledProperty<string?> SuffixProperty =
            AvaloniaProperty.Register<NumericField, string?>(nameof(Suffix));

        public static readonly StyledProperty<int> DecimalsProperty =
            AvaloniaProperty.Register<NumericField, int>(nameof(Decimals), 3);

        // CLR wrappers
        public double? Value { get => GetValue(ValueProperty); set => SetValue(ValueProperty, value); }
        public double Step { get => GetValue(StepProperty); set => SetValue(StepProperty, value); }
        public double? Min { get => GetValue(MinProperty); set => SetValue(MinProperty, value); }
        public double? Max { get => GetValue(MaxProperty); set => SetValue(MaxProperty, value); }
        public bool Wrap { get => GetValue(WrapProperty); set => SetValue(WrapProperty, value); }
        public string? Suffix { get => GetValue(SuffixProperty); set => SetValue(SuffixProperty, value); }
        public int Decimals { get => GetValue(DecimalsProperty); set => SetValue(DecimalsProperty, value); }

        // State
        private bool _scrubbing;
        private bool _pendingEdit;
        private Point _startPt;
        private double _startVal;
        private bool _updatingText;
        private KeyModifiers _spinPressMods;

        private const double ScrubThreshold = 4.0; 

        public NumericField()
        {
            InitializeComponent();

            PART_Text.LostFocus += (_, __) => ParseTextToValue();
            PART_Text.KeyDown += OnKeyDown;

            SpinLeft.PointerPressed += (_, e) => _spinPressMods = e.KeyModifiers;
            SpinRight.PointerPressed += (_, e) => _spinPressMods = e.KeyModifiers;

            SpinLeft.Click += (_, __) => StepBy(-1, _spinPressMods);
            SpinRight.Click += (_, __) => StepBy(+1, _spinPressMods);

            AddHandler(PointerPressedEvent, OnPointerPressed, handledEventsToo: true);
            AddHandler(PointerReleasedEvent, OnPointerReleased, handledEventsToo: true);
            AddHandler(PointerMovedEvent, OnPointerMoved, handledEventsToo: true);

            this.GetObservable(ValueProperty).Subscribe(_ =>
            {
                if (!_scrubbing) SyncTextFromValue();
            });

            SetEditGuard(false);
            SyncTextFromValue();
        }

        private void SetEditGuard(bool on)
        {
            PART_Text.IsHitTestVisible = !on;
            PART_Text.Focusable = !on;
        }

        // --- Blender-like: click vs scrub ---
        private void OnPointerPressed(object? s, PointerPressedEventArgs e)
        {
            if (!e.GetCurrentPoint(this).Properties.IsLeftButtonPressed) return;

            _pendingEdit = true;
            _scrubbing = false;
            _startPt = e.GetPosition(this);
            _startVal = Value ?? 0;

            SetEditGuard(true); 
            Cursor = new Cursor(StandardCursorType.SizeWestEast);
            e.Handled = true;
        }

        private void OnPointerMoved(object? s, PointerEventArgs e)
        {
            if (!_pendingEdit && !_scrubbing) return;

            var dx = e.GetPosition(this).X - _startPt.X;

            if (!_scrubbing)
            {
                if (Math.Abs(dx) >= ScrubThreshold)
                {
                    _scrubbing = true;
                    _pendingEdit = false;
                    e.Pointer.Capture(this);
                    Cursor = new Cursor(StandardCursorType.SizeWestEast);
                }
                else
                {
                    return; 
                }
            }

            var mul = KeyMul(e.KeyModifiers);
            SetValueInternal(_startVal + dx * Step * mul);
            e.Handled = true;
        }

        private void OnPointerReleased(object? s, PointerReleasedEventArgs e)
        {
            if (_scrubbing)
            {
                _scrubbing = false;
                if (e.Pointer.Captured == this) e.Pointer.Capture(null);
                Cursor = null;
                SetEditGuard(false);
                e.Handled = true;
                return;
            }

            if (_pendingEdit)
            {
                _pendingEdit = false;
                SetEditGuard(false);
                PART_Text.Focus();
                PART_Text.SelectAll();
                e.Handled = true;
            }
        }

        private void OnKeyDown(object? s, KeyEventArgs e)
        {
            if (e.Key is Key.Left or Key.Right or Key.Up or Key.Down)
            {
                var sign = (e.Key is Key.Right or Key.Up) ? 1 : -1;
                var mul = KeyMul(e.KeyModifiers);
                SetValueInternal((Value ?? 0) + sign * Step * mul);
                e.Handled = true;
            }
            else if (e.Key == Key.Enter)
            {
                ParseTextToValue();
                e.Handled = true;
            }
        }

        private void StepBy(double dir, KeyModifiers mods)
        {
            var mul = KeyMul(mods);
            SetValueInternal((Value ?? 0) + dir * Step * mul);
        }

        private static double KeyMul(KeyModifiers mods)
        {
            double m = 1.0;
            if (mods.HasFlag(KeyModifiers.Shift)) m *= 10.0;
            if (mods.HasFlag(KeyModifiers.Control)) m *= 0.1;
            return m;
        }

        // --- Value ↔ Text ---
        private void SetValueInternal(double newVal)
        {
            if (Wrap && Min.HasValue && Max.HasValue)
            {
                var min = Min.Value; var max = Max.Value; var range = max - min;
                if (Math.Abs(range) > double.Epsilon)
                    newVal = newVal - Math.Floor((newVal - min) / range) * range;
            }
            if (Min.HasValue) newVal = Math.Max(Min.Value, newVal);
            if (Max.HasValue) newVal = Math.Min(Max.Value, newVal);

            Value = newVal;
            SyncTextFromValue();
        }

        private void SyncTextFromValue()
        {
            _updatingText = true;
            try
            {
                var v = Value ?? 0;
                var s = v.ToString("F" + Math.Clamp(Decimals, 0, 8), CultureInfo.InvariantCulture);
                if (!string.IsNullOrEmpty(Suffix)) s += Suffix;
                PART_Text.Text = s;
                PART_Text.CaretIndex = PART_Text.Text?.Length ?? 0;
            }
            finally { _updatingText = false; }
        }

        private void ParseTextToValue()
        {
            if (_updatingText) return;

            var t = PART_Text.Text ?? string.Empty;
            if (!string.IsNullOrEmpty(Suffix) && t.EndsWith(Suffix, StringComparison.Ordinal))
                t = t.Substring(0, t.Length - Suffix!.Length);

            if (double.TryParse(t, NumberStyles.Float, CultureInfo.InvariantCulture, out var parsed))
                SetValueInternal(parsed);
            else
                SyncTextFromValue();
        }
    }
}
