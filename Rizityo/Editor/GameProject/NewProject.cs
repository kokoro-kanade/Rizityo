using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Printing;
using System.Runtime.Serialization;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Editor.GameProject
{
    [DataContract]
    public class ProjectTemplate
    {
        [DataMember]
        public string  ProjectType { get; set; }
        [DataMember]
        public string ProjectFileName { get; set; }
        [DataMember]
        public List<string> Folders { get; set; }
        public byte[] Screenshot { get; set; }
        public string ScreenshotFilePath { get; set; }
        public string ProjectFilePath { get; set; }
        public string TemplateFolderPath { get; set; }
    }

    class NewProject : ViewModelBase
    {
        // インストール場所からのパスを取得する
        private readonly string _projectTemplatesFolderPath = @"..\..\Editor\ProjectTemplates";

        private string _projectName = "NewProject"; //デフォルト
        public string ProjectName
        {
            get => _projectName;
            set
            {
                if(_projectName != value)
                {
                    _projectName = value;
                    IsValidProjectPath();
                    OnPropertyChanged(nameof(ProjectName));
                }
            }
        }

        private string _createProjectPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\RizityoProjects\"; //デフォルトではドキュメント\RizityoProjectsに保存
        public string CreateProjectPath
        {
            get => _createProjectPath;
            set
            {
                if (_createProjectPath != value)
                {
                    _createProjectPath = value;
                    IsValidProjectPath();
                    OnPropertyChanged(nameof(CreateProjectPath));
                }
            }
        }

        private bool _isValid;
        public bool IsValid
        {
            get => _isValid;
            set
            {
                if (_isValid != value)
                {
                    _isValid = value;
                    OnPropertyChanged(nameof(IsValid));
                }
            }
        }

        private string _errorMsg;
        public string ErrorMsg
        {
            get => _errorMsg;
            set
            {
                if (_errorMsg != value)
                {
                    _errorMsg = value;
                    OnPropertyChanged(nameof(ErrorMsg));
                }
            }
        }

        private readonly ObservableCollection<ProjectTemplate> _projectTemplates = new ObservableCollection<ProjectTemplate>();
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }

        public NewProject()
        {
            ProjectTemplates = new ReadOnlyObservableCollection<ProjectTemplate>(_projectTemplates);
            try
            {
                var templateFiles = Directory.GetFiles(_projectTemplatesFolderPath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templateFiles.Any());
                foreach (var file in templateFiles)
                {
                    var template = Serializer.FromFile<ProjectTemplate>(file);
                    template.TemplateFolderPath = Path.GetDirectoryName(file);
                    template.ScreenshotFilePath = Path.GetFullPath(Path.Combine(template.TemplateFolderPath, "Screenshot.png"));
                    template.Screenshot = File.ReadAllBytes(template.ScreenshotFilePath);
                    template.ProjectFilePath = Path.GetFullPath(Path.Combine(template.TemplateFolderPath, template.ProjectFileName));

                    _projectTemplates.Add(template);
                }
                IsValidProjectPath();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, "Failed to read project templates");
                throw;
            }
        }

        private bool IsValidProjectPath()
        {
            var path = CreateProjectPath;
            if (!Path.EndsInDirectorySeparator(path))
            {
                path += @"\";
            }
            path += $@"{ProjectName}\";
            var nameRegex = new Regex(@"^[A-Za-z_][A-Za-z0-9_]*$");

            IsValid = false;
            if (string.IsNullOrWhiteSpace(ProjectName.Trim()))
            {
                ErrorMsg = "プロジェクト名を入力してください";
            }
            else if (!nameRegex.IsMatch(ProjectName))
            {
                ErrorMsg = "プロジェクト名に不正な文字が使われています";
            }
            else if (string.IsNullOrWhiteSpace(CreateProjectPath.Trim()))
            {
                ErrorMsg = "フォルダ名を入力してください";
            }
            else if (CreateProjectPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                ErrorMsg = "フォルダ名に不正な文字が使われています";
            }
            else if (Directory.Exists(path) && Directory.EnumerateFileSystemEntries(path).Any())
            {
                ErrorMsg = "選んだフォルダは既に存在しておりかつ空ではありません";
            }
            else
            {
                ErrorMsg = string.Empty;
                IsValid = true;
            }

            return IsValid;
        }

        public string CreateProject(ProjectTemplate template)
        {
            IsValidProjectPath();
            if (!Path.EndsInDirectorySeparator(CreateProjectPath))
            {
                CreateProjectPath += @"\";
            }
            var projectFolderPath = $@"{CreateProjectPath}{ProjectName}\";

            try
            {
                if (!Directory.Exists(projectFolderPath))
                {
                    Directory.CreateDirectory(projectFolderPath);
                }

                foreach (var folder in template.Folders)
                {
                    Directory.CreateDirectory(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(projectFolderPath), folder)));
                }
                var dirInfo = new DirectoryInfo(projectFolderPath + @".Rizityo\");
                dirInfo.Attributes |= FileAttributes.Hidden;
                File.Copy(template.ScreenshotFilePath, Path.GetFullPath(Path.Combine(dirInfo.FullName, "Screenshot.png")));

                // テンプレートのプロジェクトファイルの名前とパスを変更したものを作成
                var projectXml = File.ReadAllText(template.ProjectFilePath);
                projectXml = string.Format(projectXml, ProjectName, projectFolderPath);
                var projectFilePath = Path.GetFullPath(Path.Combine(projectFolderPath, $"{ProjectName}{Project.Extension}"));
                File.WriteAllText(projectFilePath, projectXml);

                CreateMSVCSolution(template, projectFolderPath);
                
                return projectFolderPath;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, $"Failed to create {ProjectName}");
                throw;
            }
        }

        private void CreateMSVCSolution(ProjectTemplate template, string projectFolderPath)
        {
            var templateSolutionFilePath = Path.Combine(template.TemplateFolderPath, "MSVCSolution");
            var templateProjectFilePath = Path.Combine(template.TemplateFolderPath, "MSVCProject");
            Debug.Assert(File.Exists(templateSolutionFilePath));
            Debug.Assert(File.Exists(templateProjectFilePath));

            var engineAPIFolderPath = @"$(RIZITYO_ENGINE)Engine\EngineAPI\";
            Debug.Assert(Directory.Exists(engineAPIFolderPath));

            var _0 = ProjectName;
            var _1 = "{" + Guid.NewGuid().ToString().ToUpper() + "}";
            var _2 = engineAPIFolderPath;
            var _3 = "$(RIZITYO_ENGINE)";

            var solution = File.ReadAllText(templateSolutionFilePath);
            solution = string.Format(solution, _0, _1, "{" + Guid.NewGuid().ToString().ToUpper() + "}");
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectFolderPath, $"{_0}.sln")), solution);

            var project = File.ReadAllText(templateProjectFilePath);
            project = string.Format(project, _0, _1, _2, _3);
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectFolderPath, $@"Source\{_0}.vcxproj")), project);
        }

    }


    
}
