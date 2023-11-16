using Editor.GameProject;
using System;
using System.ComponentModel;
using System.Configuration;
using System.IO;

using System.Windows;


namespace Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnMainWindowLoaded;
            Closing += OnMainWindowClosing;
        }

        public static string RizityoFolderPath { get; private set; }

        private void OnMainWindowClosing(object sender, CancelEventArgs e)
        {
            if (DataContext == null)
            {
                e.Cancel = true;
                Application.Current.MainWindow.Hide();
                OpenProjectBrowserDialog();
                if (DataContext != null)
                {
                    Application.Current.MainWindow.Show();
                }
            }
            else
            {
                Closing -= OnMainWindowClosing;
                Project.Current?.Unload();
                DataContext = null;
            }
            
        }

        private void OnMainWindowLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            GetEnginePath();
            OpenProjectBrowserDialog();
        }

        private void GetEnginePath()
        {
            var path = Environment.GetEnvironmentVariable("RIZITYO_ENGINE", EnvironmentVariableTarget.User);
            if (path == null || !Directory.Exists(Path.Combine(path, @"Engine\EngineAPI")))
            {
                var dialog = new EnginePathDialog();
                if (dialog.ShowDialog() == true)
                {
                    RizityoFolderPath = dialog.RizityoFolderPath;
                    Environment.SetEnvironmentVariable("RIZITYO_ENGINE", RizityoFolderPath, EnvironmentVariableTarget.User);
                }
                else
                {
                    Application.Current.Shutdown();
                }
            }
            else
            {
                RizityoFolderPath = path;
            }
        }

        private void OpenProjectBrowserDialog()
        {
            var projectBrowser = new GameProject.ProjectBrowserDialog();
            if(projectBrowser.ShowDialog() == false || projectBrowser.DataContext == null)
            {
                Application.Current.Shutdown();
            }
            else
            {
                Project.Current?.Unload(); // 現在のプロジェクトを閉じる
                DataContext = projectBrowser.DataContext;
            }
            
        }
    }
}
