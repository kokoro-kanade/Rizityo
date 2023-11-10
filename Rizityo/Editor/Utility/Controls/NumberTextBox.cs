using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace Editor.Utility.Controls
{
    [TemplatePart(Name = "PART_textBlock", Type =typeof(TextBlock))]
    [TemplatePart(Name = "PART_textBox", Type =typeof(TextBox))]
    class NumberTextBox : Control
    {
        private double _multiplier;
        public double Speed
        {
            get => (double)GetValue(SpeedProperty);
            set => SetValue(SpeedProperty, value);
        }

        public static readonly DependencyProperty SpeedProperty
            = DependencyProperty.Register(nameof(Speed), typeof(double), typeof(NumberTextBox),
                new PropertyMetadata(1.0));

        private bool _mouseKeyDown = false;
        private bool _valueChanged = false;

        private double _mouseXStart;

        private double _originalValue;
        public string Value
        {
            get => (string)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }

        public static readonly DependencyProperty ValueProperty
            = DependencyProperty.Register(nameof(Value), typeof(string), typeof(NumberTextBox),
                new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                    new PropertyChangedCallback(OnValueChanged)));

        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as NumberTextBox).RaiseEvent(new RoutedEventArgs(ValueChangedEvent));
        }

        public event RoutedEventHandler ValueChanged
        {
            add => AddHandler(ValueChangedEvent, value);
            remove => RemoveHandler(ValueChangedEvent, value);
        }

        public static readonly RoutedEvent ValueChangedEvent
            = EventManager.RegisterRoutedEvent(nameof(ValueChanged), RoutingStrategy.Bubble,
                typeof(RoutedEventHandler), typeof(NumberTextBox));

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            if (GetTemplateChild("PART_textBlock") is TextBlock textBlock) 
            {
                textBlock.MouseLeftButtonDown += OnTextBlock_Mouse_LBD;
                textBlock.MouseLeftButtonUp += OnTextBlock_Mouse_LBU;
                textBlock.MouseMove += OnTextBlock_Mouse_Move;
            }
        }

        private void OnTextBlock_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            double.TryParse(Value, out _originalValue);

            Mouse.Capture(sender as UIElement);
            e.Handled = true;
            _mouseKeyDown = true;
            _valueChanged = false;
            _mouseXStart = e.GetPosition(this).X;
            Focus();
        }
        private void OnTextBlock_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            if (_mouseKeyDown)
            {
                Mouse.Capture(null);
                e.Handled = true;
                _mouseKeyDown = false;
                // テキストボックスに数値を入力するパターン
                if (!_valueChanged && GetTemplateChild("PART_textBox") is TextBox textBox)
                {
                    textBox.Visibility = Visibility.Visible;
                    textBox.Focus();
                    textBox.SelectAll();
                }
            }
        }
        private void OnTextBlock_Mouse_Move(object sender, MouseEventArgs e)
        {
            var mouseX = e.GetPosition(this).X;
            var dx = mouseX - _mouseXStart;
            // スライダーで数値を変更するパターン
            if (_mouseKeyDown && Math.Abs(dx) > SystemParameters.MinimumHorizontalDragDistance)
            {
                if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
                    _multiplier = 0.001;
                else if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
                    _multiplier = 0.1;
                else
                    _multiplier = 0.01;
                var newValue = _originalValue + (dx * _multiplier * Speed);
                Value = newValue.ToString("G5");
                _valueChanged = true;
            }
        }

        static NumberTextBox()
        {
            // WPF側にこのクラスをStyleで指定できるように設定
            DefaultStyleKeyProperty.OverrideMetadata(typeof(NumberTextBox),
                new FrameworkPropertyMetadata(typeof(NumberTextBox)));
        }
    }
}
