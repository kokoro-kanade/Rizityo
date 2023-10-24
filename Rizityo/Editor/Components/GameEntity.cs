using Editor.GameProject;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace Editor.Components
{
    [DataContract]
    [KnownType(typeof(TransformComponent))] // コンポーネントの継承クラスをシリアライズするため
    class GameEntity : ViewModelBase
    {
        private bool _isEnabled;
        [DataMember]
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        private string _name;
        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        [DataMember]
        public Level ParentLevel { get; private set; }

        [DataMember(Name = nameof(Components))]
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; private set; }

        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            if (_components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(_components);
                OnPropertyChanged(nameof(Components));
            }

        }

        public GameEntity(Level level)
        {
            Debug.Assert(level != null);
            ParentLevel = level;
            _components.Add(new TransformComponent(this));
            OnDeserialized(new StreamingContext());
        }
    }

    abstract class MultiSelectedEntity : ViewModelBase
    {
        private bool? _isEnabled; // 選択しているすべてのエンティティの値が等しくない場合はnullにするためにbool?になっている
        [DataMember]
        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        private string _name;
        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        private readonly ObservableCollection<IMultiSelectedComponent> _components = new ObservableCollection<IMultiSelectedComponent>();
        public ReadOnlyObservableCollection<IMultiSelectedComponent> Components { get; }

        public List<GameEntity> SelectedEntities { get; }

        // 選択しているエンティティの値が同じならその値、そうでなければnull
        public static float? GetMixedValue(List<GameEntity> entities, Func<GameEntity, float> getPropertyFunc)
        {
            var value = getPropertyFunc(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (!value.IsTheSameAs(getPropertyFunc(entity)))
                {
                    return null;
                }
            }
            return value;
        }

        public static bool? GetMixedValue(List<GameEntity> entities, Func<GameEntity, bool> getPropertyFunc)
        {
            var value = getPropertyFunc(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getPropertyFunc(entity))
                {
                    return null;
                }
            }
            return value;
        }

        public static string GetMixedValue(List<GameEntity> entities, Func<GameEntity, string> getPropertyFunc)
        {
            var value = getPropertyFunc(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getPropertyFunc(entity))
                {
                    return null;
                }
            }
            return value;
        }

        private bool _enableUpdateEntities = true;
        protected virtual bool UpdateGameEntities(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(IsEnabled):
                    SelectedEntities.ForEach(x => x.IsEnabled = IsEnabled.Value);
                    return true;
                case nameof(Name):
                    SelectedEntities.ForEach(x => x.Name = Name);
                    return true;
            }
            return false; // 派生クラスにしかないプロパティの場合
        }

        // 選択しているエンティティの共通プロパティ値を調べる
        protected virtual bool UpdateMultiSelectedEntity()
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        public void Refresh()
        {
            _enableUpdateEntities = false;
            UpdateMultiSelectedEntity();
            _enableUpdateEntities = true;
        }


        public MultiSelectedEntity(List<GameEntity> entities)
        {
            Debug.Assert(entities?.Any() == true);
            Components = new ReadOnlyObservableCollection<IMultiSelectedComponent>(_components);
            SelectedEntities = entities;
            PropertyChanged += (x, e) => { if(_enableUpdateEntities) UpdateGameEntities(e.PropertyName); }; // 選択されているエンティティのプロパティも更新
        }
    }

    class MultiSelectedGameEntity : MultiSelectedEntity
    {
        public MultiSelectedGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
