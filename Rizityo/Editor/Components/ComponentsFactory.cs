using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Components
{
    enum ComponentType
    {
        Transform,
        Script
    }

    static class ComponentsFactory
    {
        private static readonly Func<GameEntity, object, Component>[] _functions
            = new Func<GameEntity, object, Component>[]
            {
                (entity, data) => new Transform(entity),
                (entity, data) => new Script(entity){Name = (string)data}
            };

        public static Func<GameEntity, object, Component> GetCreateFunc(ComponentType componentType)
        {
            Debug.Assert((int)componentType < (int)_functions.Length);
            return _functions[(int)componentType];
        }
    }
}
