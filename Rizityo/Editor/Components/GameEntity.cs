using Editor.DLLWrapper;
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
    [KnownType(typeof(Transform))] // コンポーネントの継承クラスをシリアライズするため
    class GameEntity : ViewModelBase // Rename: -> Entity (c++と対応させるため)
    {
        private int _entityId = Id.INVALID_ID;
        public int EntityId
        {
            get => _entityId;
            set
            {
                if (_entityId != value)
                {
                    _entityId = value;
                    OnPropertyChanged(nameof(EntityId));
                }
            }
        }

        private bool _isActive;
        public bool IsActive
        {
            get => _isActive;
            set
            {
                if (_isActive != value)
                {
                    _isActive = value;
                    if (_isActive)
                    {
                        EntityId = EngineAPI.CreateGameEntity(this);
                        Debug.Assert(Id.IsValid(EntityId));
                    }
                    else if(Id.IsValid(EntityId))
                    {
                        EngineAPI.RemoveGameEntity(this);
                        EntityId = Id.INVALID_ID;
                    }
                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }

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

        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T; 

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
            _components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }
    }

    abstract class MultiSelectedEntity : ViewModelBase
    {
        private bool? _isEnabled = true; // 選択しているすべてのエンティティの値が等しくない場合はnullにするためにbool?になっている
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

        public T GetMultiSelectedComponent<T>() where T : IMultiSelectedComponent
        {
            return (T)Components.FirstOrDefault(c => c.GetType() == typeof(T));
        }


        // 選択しているエンティティの値が同じならその値、そうでなければnull
        // TODO: Utilityでも良い？ ここでしか使わないならここでも良い
        public static float? GetMixedValue<T>(List<T> objects, Func<T, float> getPropertyFunc)
        {
            var value = getPropertyFunc(objects.First());
            return objects.Skip(1).Any(o => !getPropertyFunc(o).IsTheSameAs(value)) ? (float?)null : value ;
        }

        public static bool? GetMixedValue<T>(List<T> objects, Func<T, bool> getPropertyFunc)
        {
            var value = getPropertyFunc(objects.First());
            return objects.Skip(1).Any(o => getPropertyFunc(o) != value) ? (bool?)null : value;
        }

        public static string GetMixedValue<T>(List<T> objects, Func<T, string> getPropertyFunc)
        {
            var value = getPropertyFunc(objects.First());
            return objects.Skip(1).Any(o => getPropertyFunc(o) != value) ? null : value;
        }

        private bool _enableUpdateEntities = true;
        protected virtual bool UpdateGameEntities(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(IsEnabled):
                    SelectedEntities.ForEach(e => e.IsEnabled = IsEnabled.Value);
                    return true;
                case nameof(Name):
                    SelectedEntities.ForEach(e => e.Name = Name);
                    return true;
            }
            return false; // 派生クラスにしかないプロパティの場合
        }

        protected virtual bool UpdateCommonProperty()
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        private void MakeComponentList()
        {
            _components.Clear();
            var firstEntity = SelectedEntities.FirstOrDefault();
            if (firstEntity == null)
                return;

            foreach (var component in firstEntity.Components)
            {
                var type = component.GetType();
                if (!SelectedEntities.Skip(1).Any(e => e.GetComponent(type) == null))
                {
                    Debug.Assert(Components.FirstOrDefault(c => c.GetType() == type) == null);
                    _components.Add(component.GetMultiSelectedComponent(this));
                }
            }
        }

        public void Refresh()
        {
            _enableUpdateEntities = false;
            UpdateCommonProperty();
            MakeComponentList();
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
