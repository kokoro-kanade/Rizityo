using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;


namespace Editor.Content
{
    /// <summary>
    /// SaveDialog.xaml の相互作用ロジック
    /// </summary>
    public partial class SaveDialog : Window
    {
        public string SaveFilePath { get; private set; }

        public SaveDialog()
        {
            InitializeComponent();
            Closing += OnSaveDialogClosing;
        }

        private bool IsValidFileName(out string saveFilePath)
        {
            var contentBrowser = contentBrowserView.DataContext as ContentBrowser;
            var path = contentBrowser.SelectedFolderPath;
            if(!Path.EndsInDirectorySeparator(path))
                path += @"\";

            var fileName = fileNameTextBox.Text.Trim();
            if (string.IsNullOrEmpty(fileName))
            {
                saveFilePath = string.Empty;
                return false;
            }

            if (!fileName.EndsWith(Asset.AssetFileExtension))
                fileName += Asset.AssetFileExtension;

            path += $@"{fileName}";
            var isValid = false;
            string errorMsg = string.Empty;

            if (fileName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                errorMsg = "ファイル名に不正な文字が使われています";
            }
            else if (File.Exists(path) &&
                     MessageBox.Show("ファイルが既に存在します。上書きしますか?", "ファイルを上書き", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.No)
            {
                // 何もしない
            }
            else
            {
                isValid = true;
            }

            if (!string.IsNullOrEmpty(errorMsg))
            {
                MessageBox.Show(errorMsg, "エラー", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            saveFilePath = path;
            return isValid;
        }

        private void OnSaveButton_Click(object sender, RoutedEventArgs e)
        {
            if(IsValidFileName(out var saveFilePath))
            {
                SaveFilePath = saveFilePath;
                DialogResult = true;
                Close();
            }
        }

        private void OnContentBrowser_DoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if((e.OriginalSource as FrameworkElement).DataContext == contentBrowserView.SelectedItem &&
                contentBrowserView.SelectedItem.FileName == fileNameTextBox.Text)
            {
                OnSaveButton_Click(sender, null);
            }
        }

        private void OnSaveDialogClosing(object sender, CancelEventArgs e)
        {
            contentBrowserView.Dispose();
        }
    }
}
