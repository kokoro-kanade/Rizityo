using Editor.Components;
using Editor.DLLWrapper;
using Editor.GameDev;
using Editor.Utility;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace Editor.GameProject
{
    enum BuildConfigutation
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor
    }

    [DataContract(Name = "Game")]
    class Project : ViewModelBase
    {
        public static string Extension { get; } = ".rproject";
        [DataMember]
        public string Name { get; private set; }
        [DataMember]
        public string Path { get; private set; }
        public string ProjectFilePath => $@"{Path}{Name}{Extension}";
        public string SolutionFilePath => $@"{Path}{Name}.sln";

        public static Project Current => Application.Current.MainWindow.DataContext as Project;

        // レベル
        [DataMember(Name = "Levels")]
        private ObservableCollection<Level> _levels = new ObservableCollection<Level>();
        public ReadOnlyObservableCollection<Level> Levels { get; private set; }

        private Level _activeLevel;
        public Level ActiveLevel
        {
            get => _activeLevel;
            set
            {
                if (_activeLevel != value)
                {
                    _activeLevel = value;
                    OnPropertyChanged(nameof(ActiveLevel));
                }
            }
        }
        public ICommand AddLevelCommand { get; private set; }
        public ICommand RemoveLevelCommand { get; private set; }
        private void AddLevel(string levelName)
        {
            Debug.Assert(!string.IsNullOrEmpty(levelName.Trim()));
            _levels.Add(new Level(this, levelName));
        }
        public void RemoveLevel(Level level)
        {
            Debug.Assert(_levels.Contains(level));
            _levels.Remove(level);
        }

        private string[] _availableScripts;
        public string[] AvailableScripts
        {
            get => _availableScripts;
            set
            {
                if (_availableScripts != value)
                {
                    _availableScripts = value;
                    OnPropertyChanged(nameof(AvailableScripts));
                }
            }
        }

        public static UndoRedo UndoRedo { get; } = new UndoRedo();
        public ICommand UndoCommand { get; private set; }
        public ICommand RedoCommand { get; private set; }


        public ICommand SaveCommand { get; private set; }
        public static Project Load(string projectFile)
        {
            Debug.Assert(File.Exists(projectFile));
            return Serializer.FromFile<Project>(projectFile);
        }
        public static void Save(Project project)
        {
            Serializer.ToFile(project, project.ProjectFilePath);
            Logger.Log(Verbosity.Display, $"Project saved to {project.ProjectFilePath}");
        }
        public void Unload()
        {
            UnLoadGameCodeDll();
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
        }

        // ビルド
        private int _buildConfig;
        [DataMember]
        public int BuildConfig
        {
            get => _buildConfig;
            set
            {
                if (_buildConfig != value)
                {
                    _buildConfig = value;
                    OnPropertyChanged(nameof(BuildConfig));
                }
            }
        }
        public BuildConfigutation ApplicationBuildConfig => BuildConfig == 0 ? BuildConfigutation.Debug : BuildConfigutation.Release;
        public BuildConfigutation DllBuildConfig => BuildConfig == 0 ? BuildConfigutation.DebugEditor : BuildConfigutation.ReleaseEditor;

        private static readonly string[] _buildConfigurationNames
            = new string[] { "Debug", "DebugEditor", "Release", "ReleaseEditor" };
        private static string GetConfigurationName(BuildConfigutation config) => _buildConfigurationNames[(int)config];
        public ICommand BuildCommand { get; private set; }
        private async Task BuildGameCodeDll(bool showVSWindow)
        {
            try
            {
                UnLoadGameCodeDll();
                await Task.Run(() => VisualStudio.BuildSolution(this, GetConfigurationName(DllBuildConfig), showVSWindow));
                if (VisualStudio.BuildSucceeded)
                {
                    LoadGameCodeDll();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                throw;
            }
        }
        private void LoadGameCodeDll()
        {
            var configName = GetConfigurationName(DllBuildConfig);
            var dllPath = $@"{Path}x64\{configName}\{Name}.dll";
            if(File.Exists(dllPath) && EngineAPI.LoadGameCodeDll(dllPath) != 0)
            {
                AvailableScripts = EngineAPI.GetGameScriptNames();
                ActiveLevel.GameEntities.Where(e => e.GetComponent<Script>() != null).ToList().ForEach(e => e.IsActive = true);
                Logger.Log(Verbosity.Display, "ゲームコードのDLLをロードしました");
            }
            else
            {
                Logger.Log(Verbosity.Warning, "ゲームコードのDLLのロードに失敗しました。まずプロジェクトのビルドを試してください");
            }
        }
        private void UnLoadGameCodeDll()
        {
            ActiveLevel.GameEntities.Where(e => e.GetComponent<Script>() != null).ToList().ForEach(e => e.IsActive = false);
            if(EngineAPI.UnLoadGameCodeDll() != 0)
            {
                Logger.Log(Verbosity.Display, "ゲームコードのDLLをアンロードしました");
            }
        }

        private void SetCommands()
        {
            AddLevelCommand = new RelayCommand<object>(x =>
            {
                AddLevel($"Level {_levels.Count + 1}");
                var newLevel = _levels.Last();
                var levelIndex = _levels.Count - 1;
                UndoRedo.Add(new UndoRedoAction(
                    () => RemoveLevel(newLevel),
                    () => _levels.Insert(levelIndex, newLevel),
                    $"Add {newLevel.Name}"));
            });

            RemoveLevelCommand = new RelayCommand<Level>(x =>
            {
                var levelIndex = _levels.IndexOf(x);
                RemoveLevel(x);
                UndoRedo.Add(new UndoRedoAction(
                    () => _levels.Insert(levelIndex, x),
                    () => RemoveLevel(x),
                    $"Remove {x.Name}"));
            }, x => !x.IsActive);

            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo(), x => UndoRedo.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo(), x => UndoRedo.RedoList.Any());
            SaveCommand = new RelayCommand<object>(x => Save(this));
            BuildCommand = new RelayCommand<bool>(async x => await BuildGameCodeDll(x), x => !(VisualStudio.IsDebugging() && VisualStudio.BuildDone));

            OnPropertyChanged(nameof(AddLevelCommand));
            OnPropertyChanged(nameof(RemoveLevelCommand));
            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(SaveCommand));
            OnPropertyChanged(nameof(BuildCommand));
        }

        // ロード時にコンストラクタを用いずに初期化するための処理
        [OnDeserialized]
        private async void OnDeserialized(StreamingContext context)
        {
            if (_levels != null)
            {
                Levels = new ReadOnlyObservableCollection<Level>(_levels);
                OnPropertyChanged(nameof(Levels));
            }
            ActiveLevel = Levels.FirstOrDefault(x => x.IsActive);

            await BuildGameCodeDll(false);

            SetCommands();
        }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;


            OnDeserialized(new StreamingContext());
        }


    }
}
