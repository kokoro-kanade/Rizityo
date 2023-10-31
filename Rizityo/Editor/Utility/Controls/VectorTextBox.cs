using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace Editor.Utility.Controls
{
    public enum VectorType
    {
        Vector2,
        Vector3,
        Vector4
    }

    class VectorTextBox : Control
    {
        public Orientation Orientation
        {
            get => (Orientation)GetValue(OrientationProperty);
            set => SetValue(OrientationProperty, value);
        }

        public static readonly DependencyProperty OrientationProperty
            = DependencyProperty.Register(nameof(Orientation), typeof(Orientation), typeof(VectorTextBox),
                new PropertyMetadata(Orientation.Horizontal));

        public VectorType VectorType
        {
            get => (VectorType)GetValue(VectorTypeProperty);
            set => SetValue(VectorTypeProperty, value);
        }

        public static readonly DependencyProperty VectorTypeProperty
            = DependencyProperty.Register(nameof(VectorType), typeof(VectorType), typeof(VectorTextBox),
                new PropertyMetadata(VectorType.Vector3));


        public double X
        {
            get => (double)GetValue(XProperty);
            set => SetValue(XProperty, value);
        }

        public static readonly DependencyProperty XProperty
            = DependencyProperty.Register(nameof(X), typeof(double), typeof(VectorTextBox),
                new PropertyMetadata(1.0));

        public double Y
        {
            get => (double)GetValue(YProperty);
            set => SetValue(YProperty, value);
        }

        public static readonly DependencyProperty YProperty
            = DependencyProperty.Register(nameof(Y), typeof(double), typeof(VectorTextBox),
                new PropertyMetadata(1.0));

        public double Z
        {
            get => (double)GetValue(ZProperty);
            set => SetValue(ZProperty, value);
        }

        public static readonly DependencyProperty ZProperty
            = DependencyProperty.Register(nameof(Z), typeof(double), typeof(VectorTextBox),
                new PropertyMetadata(1.0));

        public double W
        {
            get => (double)GetValue(WProperty);
            set => SetValue(WProperty, value);
        }

        public static readonly DependencyProperty WProperty
            = DependencyProperty.Register(nameof(W), typeof(double), typeof(VectorTextBox),
                new PropertyMetadata(1.0));

        public double Speed
        {
            get => (double)GetValue(SpeedProperty);
            set => SetValue(SpeedProperty, value);
        }

        public static readonly DependencyProperty SpeedProperty
            = DependencyProperty.Register(nameof(Speed), typeof(double), typeof(VectorTextBox),
                new PropertyMetadata(1.0));

        static VectorTextBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(VectorTextBox),
                new FrameworkPropertyMetadata(typeof(VectorTextBox)));
        }
    }
}
