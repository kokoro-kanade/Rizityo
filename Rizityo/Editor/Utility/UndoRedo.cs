﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Animation;

namespace Editor.Utility
{
    public interface IUndoRedo
    {
        string Name { get; }
        void Undo();
        void Redo();
    }

    public class UndoRedoAction : IUndoRedo
    {
        private Action _undoAction;
        private Action _redoAction;
        public string Name { get; }
        public void Undo() => _undoAction();
        public void Redo() => _redoAction();

        public UndoRedoAction(string name) { Name = name; }
        public UndoRedoAction(Action undo, Action redo, string name) : this(name)
        {
            Debug.Assert(undo != null && redo != null);
            _undoAction = undo;
            _redoAction = redo;
        }
        public UndoRedoAction(string property, object instance, object undoValue, object redoValue, string name) :
            this(
                () => instance.GetType().GetProperty(property).SetValue(instance, undoValue),
                () => instance.GetType().GetProperty(property).SetValue(instance, redoValue),
                name)
        {}
    }

    public class UndoRedo
    {
        // UIではundoLIstの次にredoListを表示するのでundoは末尾を操作しredoは先頭を操作
        private readonly ObservableCollection<IUndoRedo> _undoList = new ObservableCollection<IUndoRedo>(); 
        private readonly ObservableCollection<IUndoRedo> _redoList = new ObservableCollection<IUndoRedo>(); 
        public ReadOnlyObservableCollection<IUndoRedo> UndoList { get; }
        public ReadOnlyObservableCollection<IUndoRedo> RedoList { get; }

        private bool _enableAdd = true;

        public void Reset()
        {
            _undoList.Clear();
            _redoList.Clear();
        }

        public void Add(IUndoRedo cmd)
        {
            if (_enableAdd)
            {
                _undoList.Add(cmd);
                _redoList.Clear();
            }
        }

        public void Undo()
        {
            if (_undoList.Any())
            {
                var cmd = _undoList.Last();
                _undoList.RemoveAt(_undoList.Count - 1);
                _enableAdd = false;
                cmd.Undo();
                _enableAdd = true;
                _redoList.Insert(0, cmd);
            }
        }

        public void Redo()
        {
            if (_redoList.Any())
            {
                var cmd = _redoList.First();
                _redoList.RemoveAt(0);
                _enableAdd = false;
                cmd.Redo();
                _enableAdd = true;
                _undoList.Add(cmd);
            }
        }

        public UndoRedo()
        {
            UndoList = new ReadOnlyObservableCollection<IUndoRedo>(_undoList);
            RedoList = new ReadOnlyObservableCollection<IUndoRedo>(_redoList);
        }
    }
}
