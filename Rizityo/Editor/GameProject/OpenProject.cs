using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

namespace Editor.GameProject
{
    [DataContract]
    public class ProjectData
    {
        [DataMember]
        public string Name { get; set; }
        [DataMember]
        public string Path { get; set; }
        [DataMember]
        public DateTime Date { get; set; }
        public string FullPath { get => $"{Path}{Name}{Project.Extension}"; }
        public byte[] Screenshot { get; set; }
    }

    [DataContract]
    public class ProjectDataList
    {
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }

    class OpenProject
    {
        // \User\AppData\Roaming\Rizityo
        private static readonly string _applicaionDataPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\Rizityo\";
        private static readonly string _projectDataPath;
        private static readonly ObservableCollection<ProjectData> _projects = new ObservableCollection<ProjectData>(); //readonlyなので_projectsへの代入はできないが_projectsの参照するリストは操作可能
        public static ReadOnlyObservableCollection<ProjectData> Projects { get; } //エディタで表示する用


        private static void ReadProjectData()
        {
            if (File.Exists(_projectDataPath))
            {
                var projects = Serializer.FromFile<ProjectDataList>(_projectDataPath).Projects.OrderByDescending(x => x.Date);
                _projects.Clear();
                foreach (var project in projects)
                {
                    if (!File.Exists(project.FullPath)) // プロジェクトが削除されている可能性を考慮
                        continue;

                    project.Screenshot = File.ReadAllBytes($@"{project.Path}\.Rizityo\Screenshot.png");
                    _projects.Add(project);
                }
            }
            
        }

        private static void WriteProjectData()
        {
            var projects = _projects.OrderBy(x => x.Date).ToList();
            Serializer.ToFile(new ProjectDataList() { Projects = projects }, _projectDataPath);
        }

        public static Project Open(ProjectData openProject)
        {
            ReadProjectData(); // 常に最新の状態を読み込む(エディタが複数開かれる場合等のため)
            var project = _projects.FirstOrDefault(x => x.FullPath == openProject.FullPath);
            if(project != null)
            {
                project.Date = DateTime.Now;
            }
            else // 新規作成時
            {
                project = openProject;
                project.Date = DateTime.Now;
                _projects.Add(project);
            }
            WriteProjectData();
            return Project.Load(project.FullPath);
        }

        static OpenProject()
        {
            try
            {
                if (!Directory.Exists(_applicaionDataPath))
                {
                    Directory.CreateDirectory(_applicaionDataPath);
                }
                _projectDataPath = $@"{_applicaionDataPath}ProjectData.xml";
                Projects = new ReadOnlyObservableCollection<ProjectData>(_projects);
                ReadProjectData();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, "Failed to read project data");

            }
        }
    }
}
