using Editor.GameDev;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Dynamic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace Editor.GameProject
{
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

        public static Project Current => Application.Current.MainWindow.DataContext as Project;
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
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
        }

        // ロード時にコンストラクタを用いずに初期化するための処理
        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (_levels != null)
            {
                Levels = new ReadOnlyObservableCollection<Level>(_levels);
                OnPropertyChanged(nameof(Levels));
            }
            ActiveLevel = Levels.FirstOrDefault(x => x.IsActive);

            AddLevelCommand = new RelayCommand<object>(x =>
            {
                AddLevel($"Level {_levels.Count+1}");
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

            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo());
            SaveCommand = new RelayCommand<object>(x => Save(this));
        }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;


            OnDeserialized(new StreamingContext());
        }


    }
}
