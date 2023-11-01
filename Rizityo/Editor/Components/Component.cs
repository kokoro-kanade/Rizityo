using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Components
{
    [DataContract]
    abstract class Component : ViewModelBase
    {

        [DataMember]
        public GameEntity Owner { get; private set; }
        public abstract IMultiSelectedComponent GetMultiSelectedComponent(MultiSelectedEntity msEntity);

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    interface IMultiSelectedComponent { }

    // ジェネリック型のクラスをリストには持てないので空のインターフェースを継承させる
    abstract class MultiSelectedComponent<T> : ViewModelBase, IMultiSelectedComponent where T : Component
    {
        public List<T> SelectedComponents { get; }

        private bool _enableUpdateComponents = true;
        protected abstract bool UpdateComponents(string propertyName);
        protected abstract bool UpdateCommonProperty();

        public void Refresh()
        {
            _enableUpdateComponents = false;
            UpdateCommonProperty();
            _enableUpdateComponents = true;
        }

        public MultiSelectedComponent(MultiSelectedEntity msEntity)
        {
            Debug.Assert(msEntity?.SelectedEntities?.Any() == true);
            SelectedComponents = msEntity.SelectedEntities.Select(entity => entity.GetComponent<T>()).ToList();
            PropertyChanged += (s, e) => { if (_enableUpdateComponents) UpdateComponents(e.PropertyName);  };
        }

    }


}
