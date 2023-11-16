using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.IO;
using Editor.GameProject;
using System.Windows.Media;
using System.Diagnostics;
using Editor.Utility;
using System.Text.RegularExpressions;

namespace Editor.GameDev
{
    /// <summary>
    /// NewScriptDialog.xaml の相互作用ロジック
    /// </summary>
    public partial class NewScriptDialog : Window
    {
        private static readonly string _cppCode = @"#include ""{0}.h""

namespace {1}
{{
    REGISTER_SCRIPT({0});

    void {0}::BeginPlay()
    {{

    }}

    void {0}::Update(float dt)
    {{

    }}
}}";
        private static readonly string _headerCode = @"#pragma once
#include <string>

namespace {1}
{{
	class {0} : public Rizityo::Script::EntityScript
	{{
	public:
		constexpr explicit {0}(Rizityo::GameEntity::Entity entity) : Rizityo::Script::EntityScript(entity) {{}}

		void BeginPlay() override;
		void Update(float dt) override;

	}};

}}";

        private static readonly string _namespace = GetNamespaceFromProjectName();

        private static string GetNamespaceFromProjectName()
        {
            var projectName = Project.Current.Name.Trim();
            if (string.IsNullOrEmpty(projectName))
                return string.Empty;

            projectName = Regex.Replace(projectName, @"[^A-Za-z0-9_]", "");
            return projectName;
        }

        public NewScriptDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
            folderTextBox.Text = @"Source\";
        }

        private bool IsValid()
        {
            bool isValid = false;
            var fileName = fileTextBox.Text.Trim();
            var folderPath = folderTextBox.Text.Trim();
            string errorMsg = string.Empty;
            var nameRegex = new Regex(@"^[A-Za-z_][A-Za-z0-9_]*$");

            if (string.IsNullOrEmpty(fileName))
            {
                errorMsg = "ファイル名を入力してください";
            }
            else if (!nameRegex.IsMatch(fileName))
            {
                errorMsg = "ファイル名に不正な文字が使われています";
            }
            else if (string.IsNullOrWhiteSpace(folderPath))
            {
                errorMsg = "フォルダを選択してください";
            }
            else if (folderPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                errorMsg = "フォルダ名に不正な文字が使われています";
            }
            else if (!Path.GetFullPath(Path.Combine(Project.Current.Path, folderPath)).Contains(Path.Combine(Project.Current.Path, @"Source\")))
            {
                errorMsg = "ファイルはSourceフォルダに追加してください";
            }
            else if (File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, folderPath), $"{fileName}.cpp"))) ||
                File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, folderPath), $"{fileName}.h"))))
            {
                errorMsg = $"ファイル{fileName}は既に存在しています";
            }
            else
            {
                isValid = true;
            }

            if (!isValid)
            {
                messageTextBlock.Foreground = FindResource("Editor.RedBrush") as Brush;
            }
            else
            {
                messageTextBlock.Foreground = FindResource("Editor.FontBrush") as Brush;
            }
            messageTextBlock.Text = errorMsg;
            return isValid;
        }

        private void OnFileTextBox_TextChandged(object sender, TextChangedEventArgs e)
        {
            if (!IsValid())
                return;
            var fileName = fileTextBox.Text.Trim();
            messageTextBlock.Text = $"{fileName}.hと{fileName}.cppが{Project.Current.Name}に追加されます";
        }

        private void OnFolderTextBox_TextChandged(object sender, TextChangedEventArgs e)
        {
            IsValid();
        }

        private async void OnCreateButton_Click(object sender, RoutedEventArgs e)
        {
            if (!IsValid())
                return;
            IsEnabled = false;
            busyTextBlock.Visibility = Visibility.Visible;

            try
            {
                var fileName = fileTextBox.Text.Trim();
                var folderPath = Path.GetFullPath(Path.Combine(Project.Current.Path, folderTextBox.Text.Trim()));
                var solutionFilePath = Project.Current.SolutionFilePath;
                var projectName = Project.Current.Name;
                await Task.Run(() => CreateScript(fileName, folderPath, solutionFilePath, projectName));
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, $"ファイル{fileTextBox.Text}を作成できませんでした");
            }
            finally
            {
                busyTextBlock.Visibility = Visibility.Hidden;
                Close();
            }
        }

        private void CreateScript(string fileName, string folderPath, string solutionFilePath, string projectName)
        {
            if (!Directory.Exists(folderPath))
                Directory.CreateDirectory(folderPath);

            var cppPath = Path.GetFullPath(Path.Combine(folderPath, $"{fileName}.cpp"));
            var headerPath = Path.GetFullPath(Path.Combine(folderPath, $"{fileName}.h"));

            using(var sw = File.CreateText(cppPath))
            {
                sw.Write(string.Format(_cppCode, fileName, _namespace));
            }
            using (var sw = File.CreateText(headerPath))
            {
                sw.Write(string.Format(_headerCode, fileName, _namespace));
            }

            string[] files = new string[] { cppPath, headerPath };
            for (int i = 0; i < 3; i++)
            {
                // 失敗したら時間を待ってトライ
                if (!VisualStudio.AddFilesToSolution(solutionFilePath, projectName, files))
                    System.Threading.Thread.Sleep(1000);
                else
                    break;
            }
        }
    }
}
