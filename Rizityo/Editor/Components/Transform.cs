using Editor.Utility;
using System;
using System.Collections.Generic;
using System.DirectoryServices.ActiveDirectory;
using System.Linq;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Components
{ 

    [DataContract]
    class Transform : Component
    {
        private Vector3 _position;
        [DataMember]
        public Vector3 Position
        {
            get => _position;
            set
            {
                if (_position != value)
                {
                    _position = value;
                    OnPropertyChanged(nameof(Position));
                }
            }
        }

        private Vector3 _rotation;
        [DataMember]
        public Vector3 Rotation
        {
            get => _rotation;
            set
            {
                if (_rotation != value)
                {
                    _rotation = value;
                    OnPropertyChanged(nameof(Rotation));
                }
            }
        }

        private Vector3 _scale;
        [DataMember]
        public Vector3 Scale
        {
            get => _scale;
            set
            {
                if (_scale != value)
                {
                    _scale = value;
                    OnPropertyChanged(nameof(Scale));
                }
            }
        }

        public override IMultiSelectedComponent GetMultiSelectedComponent(MultiSelectedEntity msEntity)
            => new MultiSelectedTransform(msEntity);

        public Transform(GameEntity owner) : base(owner)
        {
        }
 
    }

    sealed class MultiSelectedTransform : MultiSelectedComponent<Transform>
    {
        // x,y,zのどれかを統一して変更することができるように全成分を持つ
        private float? _posX;
        public float? PosX
        {
            get => _posX;
            set
            {
                if (!MathUtil.IsTheSameAs(_posX, value))
                {
                    _posX = value;
                    OnPropertyChanged(nameof(PosX));
                }
            }
        }

        private float? _posY;
        public float? PosY
        {
            get => _posY;
            set
            {
                if (!MathUtil.IsTheSameAs(_posY, value))
                {
                    _posY = value;
                    OnPropertyChanged(nameof(PosY));
                }
            }
        }

        private float? _posZ;
        public float? PosZ
        {
            get => _posZ;
            set
            {
                if (!MathUtil.IsTheSameAs(_posZ, value))
                {
                    _posZ = value;
                    OnPropertyChanged(nameof(PosZ));
                }
            }
        }

        private float? _rotX;
        public float? RotX
        {
            get => _rotX;
            set
            {
                if (!MathUtil.IsTheSameAs(_rotX, value))
                {
                    _rotX = value;
                    OnPropertyChanged(nameof(RotX));
                }
            }
        }

        private float? _rotY;
        public float? RotY
        {
            get => _rotY;
            set
            {
                if (!MathUtil.IsTheSameAs(_rotY, value))
                {
                    _rotY = value;
                    OnPropertyChanged(nameof(RotY));
                }
            }
        }

        private float? _rotZ;
        public float? RotZ
        {
            get => _rotZ;
            set
            {
                if (!MathUtil.IsTheSameAs(_rotZ, value))
                {
                    _rotZ = value;
                    OnPropertyChanged(nameof(RotZ));
                }
            }
        }

        private float? _scaleX;
        public float? ScaleX
        {
            get => _scaleX;
            set
            {
                if (!MathUtil.IsTheSameAs(_scaleX, value))
                {
                    _scaleX = value;
                    OnPropertyChanged(nameof(ScaleX));
                }
            }
        }

        private float? _scaleY;
        public float? ScaleY
        {
            get => _scaleY;
            set
            {
                if (!MathUtil.IsTheSameAs(_scaleY, value))
                {
                    _scaleY = value;
                    OnPropertyChanged(nameof(ScaleY));
                }
            }
        }

        private float? _scaleZ;
        public float? ScaleZ
        {
            get => _scaleZ;
            set
            {
                if (!_scaleZ.IsTheSameAs(value))
                {
                    _scaleZ = value;
                    OnPropertyChanged(nameof(ScaleZ));
                }
            }
        }

        protected override bool UpdateComponents(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(PosX):
                case nameof(PosY):
                case nameof(PosZ):
                    SelectedComponents.ForEach(t => t.Position = new Vector3(PosX ?? t.Position.X, PosY ?? t.Position.Y, PosZ ?? t.Position.Z));
                    return true;

                case nameof(RotX):
                case nameof(RotY):
                case nameof(RotZ):
                    SelectedComponents.ForEach(t => t.Rotation = new Vector3(RotX ?? t.Rotation.X, RotY ?? t.Rotation.Y, RotZ ?? t.Rotation.Z));
                    return true;

                case nameof(ScaleX):
                case nameof(ScaleY):
                case nameof(ScaleZ):
                    SelectedComponents.ForEach(t => t.Scale = new Vector3(ScaleX ?? t.Scale.X, ScaleY ?? t.Scale.Y, ScaleZ ?? t.Scale.Z));
                    return true;
            }
            return false;
        }

        protected override bool UpdateCommonProperty()
        {
            PosX = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Position.X));
            PosY = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Position.Y));
            PosZ = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Position.Z));
            RotX = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Rotation.X));
            RotY = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Rotation.Y));
            RotZ = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Rotation.Z));
            ScaleX = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Scale.X));
            ScaleY = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Scale.Y));
            ScaleZ = MultiSelectedEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(t => t.Scale.Z));

            return true;
        }

        public MultiSelectedTransform(MultiSelectedEntity msEntity) : base(msEntity)
        {
            Refresh();
        }
    }
}
