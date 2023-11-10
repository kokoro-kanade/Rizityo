using Editor.GameDev;
using Editor.Content;
using System.Windows;
using System.Windows.Controls;


namespace Editor.Editors
{
    /// <summary>
    /// LevelEditorView.xaml の相互作用ロジック
    /// </summary>
    public partial class LevelEditorView : UserControl
    {
        public LevelEditorView()
        {
            InitializeComponent();
            Loaded += OnLevelEditorViewLoaded;
        }

        private void OnLevelEditorViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnLevelEditorViewLoaded;
            Focus();
        }

        private void OnNewScriptButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new NewScriptDialog();
            dialog.ShowDialog();
        }

        private void OnCreatePrimitiveMeshButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new PrimitiveMeshDialog();
            dialog.ShowDialog();
        }
    }
}
