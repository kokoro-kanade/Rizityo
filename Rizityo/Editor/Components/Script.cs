using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Components
{
    [DataContract]
    class Script : Component
    {

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

        public override IMultiSelectedComponent GetMultiSelectedComponent(MultiSelectedEntity msEntity)
            => new MultiSelectedScript(msEntity);

        public Script(GameEntity owner) : base(owner) { }
    }

    sealed class MultiSelectedScript : MultiSelectedComponent<Script>
    {
        private string _name;
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

        protected override bool UpdateCommonProperty()
        {
            Name = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Script, string>(s => s.Name));
            return true;
        }

        protected override bool UpdateComponents(string propertyName)
        {
            if (propertyName == nameof(Name))
            {
                SelectedComponents.ForEach(s => s.Name = _name);
                return true;
            }

            return false;
        }

        public MultiSelectedScript(MultiSelectedEntity msEntity) : base(msEntity)
        {
            Refresh();
        }
    }
}
