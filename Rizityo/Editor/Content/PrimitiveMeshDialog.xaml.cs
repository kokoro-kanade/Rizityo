using Editor.DLLWrapper;
using Editor.Editors;
using Editor.ToolAPIStructs;
using Editor.Utility.Controls;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Editor.Content
{
    /// <summary>
    /// PrimitiveMeshDialog.xaml の相互作用ロジック
    /// </summary>
    public partial class PrimitiveMeshDialog : Window
    {
        private static readonly List<ImageBrush> _textures = new List<ImageBrush>();

        public PrimitiveMeshDialog()
        {
            InitializeComponent();
            Loaded += (s, e) => UpdatePrimitive();
        }

        private float Value(ScalarTextBox scalarTextBox, float min)
        {
            float.TryParse(scalarTextBox.Value, out var result);
            return Math.Max(result, min);
        } // Rename

        private void UpdatePrimitive()
        {
            if (!IsInitialized)
                return;

            var primitiveType = (PrimitiveMeshType)primitiveTypeComboBox.SelectedItem;
            var info = new PrimitiveInitInfo() { Type = primitiveType };
            var smoothingAngle = 0;

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    {
                        info.SegmentX = (int)xSliderPlane.Value;
                        info.SegmentZ = (int)zSliderPlane.Value;
                        info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                        info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                        break;
                    }
                case PrimitiveMeshType.Cube:
                    return;
                case PrimitiveMeshType.UVSphere:
                    {
                        info.SegmentX = (int)lonSliderUVSphere.Value;
                        info.SegmentY = (int)latSliderUVSphere.Value;
                        info.Size.X = Value(xScalarBoxUVSphere, 0.001f);
                        info.Size.Y = Value(yScalarBoxUVSphere, 0.001f);
                        info.Size.Z = Value(zScalarBoxUVSphere, 0.001f);
                        smoothingAngle = (int)angleSliderUVSphere.Value;
                    }
                    break;
                case PrimitiveMeshType.IcoSphere:
                    return;
                case PrimitiveMeshType.Cylinder:
                    return;
                case PrimitiveMeshType.Capsule:
                    return;
                case PrimitiveMeshType.Count:
                    return;
                default:
                    return;
            }

            var geometry = new Geometry();
            geometry.ImportSetting.SmoothingAngle = smoothingAngle;
            AssetToosAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
            OnTextureCheckBox_Click(textureCheckBox, null);
        }

        private void OnPrimitiveTypeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitive();

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitive();

        private void OnScalaraBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitive();

        private void OnTextureCheckBox_Click(object sender, RoutedEventArgs e)
        {
            Brush brush = Brushes.White;
            if ((sender as CheckBox).IsChecked == true)
            {
                brush = _textures[(int)primitiveTypeComboBox.SelectedItem];
            }

            var vm = DataContext as GeometryEditor;
            foreach (var mesh in vm.MeshRenderer.Meshes)
            {
                mesh.Diffuse = brush;
            }
        }

        private static void LoadTextures()
        {
            var uris = new List<Uri>
            {
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/gandam.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/gandam.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/UVTest.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/UVTest.png"),
            };

            _textures.Clear();

            foreach (var uri in uris)
            {
                var resource = Application.GetResourceStream(uri);
                using var reader = new BinaryReader(resource.Stream);
                var data = reader.ReadBytes((int)resource.Stream.Length);
                var imageSource = (BitmapSource)new ImageSourceConverter().ConvertFrom(data);
                imageSource.Freeze();
                var brush = new ImageBrush(imageSource);
                brush.Transform = new ScaleTransform(1, -1, 0.5, 0.5);
                brush.ViewportUnits = BrushMappingMode.Absolute;
                _textures.Add(brush);
            }
        }

        static PrimitiveMeshDialog()
        {
            LoadTextures();
        }

        
    }
}
