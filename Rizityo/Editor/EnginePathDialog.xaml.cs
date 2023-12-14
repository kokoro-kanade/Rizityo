using System.IO;
using System.Windows;


namespace Editor
{
    /// <summary>
    /// EnginePathDialog.xaml の相互作用ロジック
    /// </summary>
    public partial class EnginePathDialog : Window
    {
        public string RizityoFolderPath { get; private set; }
        public EnginePathDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
        }

        private void OnOkButton_Click(object sender, RoutedEventArgs e)
        {
            var path = pathTextBox.Text.Trim();
            messageTextBlock.Text = string.Empty;
            if (string.IsNullOrEmpty(path))
            {
                messageTextBlock.Text = "パスを入力してください";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                messageTextBlock.Text = "パスに不正な文字が使われています";
            }
            else if (!Directory.Exists(Path.Combine(path, @"Engine\API\")))
            {
                messageTextBlock.Text = "指定された場所にエンジンが見つかりません";
            }

            if (string.IsNullOrEmpty(messageTextBlock.Text))
            {
                if (!Path.EndsInDirectorySeparator(path))
                    path += @"\";
                RizityoFolderPath = path;
                DialogResult = true;
                Close();
            }

        }
    }
}
