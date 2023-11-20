using System;
using Editor.Editors;
using System.Windows;
using System.Windows.Controls;


namespace Editor.Editors
{
    /// <summary>
    /// GeometryDetailView.xaml の相互作用ロジック
    /// </summary>
    public partial class GeometryDetailView : UserControl
    {
        public GeometryDetailView()
        {
            InitializeComponent();
        }

        private void OnHighlightCheckBox_Click(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as GeometryEditor;
            foreach (var m in vm.MeshRenderer.Meshes)
            {
                m.IsHighlighted = false;
            }

            var checkBox = sender as CheckBox;
            (checkBox.DataContext as MeshRendererVertexData).IsHighlighted = checkBox.IsChecked == true;
        }

        private void OnIsolateCheckBox_Click(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as GeometryEditor;
            foreach (var m in vm.MeshRenderer.Meshes)
            {
                m.IsIsolated = false;
            }

            var checkBox = sender as CheckBox;
            var mesh = checkBox.DataContext as MeshRendererVertexData;
            mesh.IsIsolated = checkBox.IsChecked == true;

            if (Tag is GeometryView geometryView)
            {
                geometryView.SetGeometry(mesh.IsIsolated ? vm.MeshRenderer.Meshes.IndexOf(mesh) : -1);
            }

        }
    }
}
