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
    [DataContract(Name = "Game")]
    class Project : ViewModelBase
    {
        public static string Extension => ".rproject";
        [DataMember]
        public string Name { get; private set; }
        [DataMember]
        public string Path { get; private set; }
        public string ProjectFilePath => $@"{Path}{Name}{Extension}";
        public string SolutionFilePath => $@"{Path}{Name}.sln";
        public string ContentFolderPath => $@"{Path}Content\";
        public string TmpFolder => $@"{Path}.Rizityo\Tmp\";
        public static Project Current => Application.Current.MainWindow?.DataContext as Project;

        // レベル
        [DataMember(Name = nameof(Levels))]
        private readonly ObservableCollection<Level> _levels = new ObservableCollection<Level>();
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


        public static Project Load(string projectFile)
        {
            Debug.Assert(File.Exists(projectFile));
            return Serializer.FromFile<Project>(projectFile);
        }

        private void DeleteTmpFolder()
        {
            if(Directory.Exists(TmpFolder))
            {
                Directory.Delete(TmpFolder, true);
            }
        }

        public void Unload()
        {
            UnLoadGameCodeDLL();
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
            Logger.Clear();
            DeleteTmpFolder();
        }

        public ICommand SaveCommand { get; private set; }
        private static void Save(Project project)
        {
            Serializer.ToFile(project, project.ProjectFilePath);
            Logger.Log(Verbosity.Display, $"Project saved to {project.ProjectFilePath}");
        }
        private void SaveToBinary()
        {
            var configName = VisualStudio.GetConfigurationName(ApplicationBuildConfig);
            var bin = $@"{Path}x64\{configName}\game.bin";

            using (var bw = new BinaryWriter(File.Open(bin, FileMode.Create, FileAccess.Write)))
            {
                bw.Write(ActiveLevel.GameEntities.Count);
                foreach (var entity in ActiveLevel.GameEntities)
                {
                    bw.Write(0); // エンティティのタイプ
                    bw.Write(entity.Components.Count);
                    foreach (var component in entity.Components)
                    {
                        bw.Write((int)component.ToEnumType());
                        component.WriteToBinary(bw);
                    }
                }
            }
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
        public BuildConfigutation DLLBuildConfig => BuildConfig == 0 ? BuildConfigutation.DebugEditor : BuildConfigutation.ReleaseEditor;

        public ICommand BuildCommand { get; private set; }
        private async Task BuildGameCodeDLL(bool showVSWindow)
        {
            try
            {
                UnLoadGameCodeDLL();
                await Task.Run(() => VisualStudio.BuildSolution(this, DLLBuildConfig, showVSWindow));
                if (VisualStudio.BuildSucceeded)
                {
                    LoadGameCodeDLL();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                throw;
            }
        }
        private void LoadGameCodeDLL()
        {
            var configName = VisualStudio.GetConfigurationName(DLLBuildConfig);
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
        private void UnLoadGameCodeDLL()
        {
            ActiveLevel.GameEntities.Where(e => e.GetComponent<Script>() != null).ToList().ForEach(e => e.IsActive = false);
            if(EngineAPI.UnLoadGameCodeDll() != 0)
            {
                Logger.Log(Verbosity.Display, "ゲームコードのDLLをアンロードしました");
            }
        }

        // デバッグ
        public ICommand DebugStartCommand { get; private set; }
        public ICommand DebugStartWithoutDebuggingCommand { get; private set; }
        public ICommand DebugStopCommand { get; private set; }
        private async Task RunGame(bool debug)
        {
            await Task.Run(() => VisualStudio.BuildSolution(this, ApplicationBuildConfig, debug));
            if (VisualStudio.BuildSucceeded)
            {
                SaveToBinary();
                await Task.Run(() => VisualStudio.Run(this, ApplicationBuildConfig, debug));
            }
        }
        private async Task StopGame() => await Task.Run(() => VisualStudio.Stop());

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
            BuildCommand = new RelayCommand<bool>(async x => await BuildGameCodeDLL(x), x => !(VisualStudio.IsDebugging() && VisualStudio.BuildDone));
            DebugStartCommand = new RelayCommand<object>(async x => await RunGame(true), x => !(VisualStudio.IsDebugging() && VisualStudio.BuildDone));
            DebugStartWithoutDebuggingCommand = new RelayCommand<object>(async x => await RunGame(false), x => !(VisualStudio.IsDebugging() && VisualStudio.BuildDone));
            DebugStopCommand = new RelayCommand<object>(async x => await StopGame(), x => VisualStudio.IsDebugging());

            OnPropertyChanged(nameof(AddLevelCommand));
            OnPropertyChanged(nameof(RemoveLevelCommand));
            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(SaveCommand));
            OnPropertyChanged(nameof(BuildCommand));
            OnPropertyChanged(nameof(DebugStartCommand));
            OnPropertyChanged(nameof(DebugStartWithoutDebuggingCommand));
            OnPropertyChanged(nameof(DebugStopCommand));

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

            await BuildGameCodeDLL(false);

            SetCommands();
        }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;

            Debug.Assert(File.Exists((Path + Name + Extension).ToLower()));
            OnDeserialized(new StreamingContext());
        }


    }
}
