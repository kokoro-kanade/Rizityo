using Editor.GameDev;
using Editor.Content;
using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

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

        private void OnNewProject(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            ProjectBrowserDialog.MoveNewProjectTab = true;
            Project.Current?.Unload();
            Application.Current.MainWindow.DataContext = null;
            Application.Current.MainWindow.Close();
        }

        private void OnOpenProject(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            Project.Current?.Unload();
            Application.Current.MainWindow.DataContext = null;
            Application.Current.MainWindow.Close();
        }

        private void OnCloseEditor(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            Application.Current.MainWindow.Close();
        }
    }
}
