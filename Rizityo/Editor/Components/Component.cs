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

    }
}
