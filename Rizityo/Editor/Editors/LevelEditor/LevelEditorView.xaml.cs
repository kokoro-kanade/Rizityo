using Editor.GameDev;
using Editor.GameProject;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

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
    }
}
