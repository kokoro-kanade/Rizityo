using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Reflection.Metadata.Ecma335;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace Editor.Utility
{
    enum Verbosity
    {
        Error = 0x01,
        Warning = 0x02,
        Display = 0x04
    }

    class LogMessage
    {
        public DateTime Time { get; }
        public Verbosity Verbosity { get; }
        public string Message { get; }
        public string File { get; }
        public string Caller { get; }
        public int Line { get; }
        public string MetaData => $"{File}: {Caller} ({Line}行目)";
        public LogMessage(Verbosity verbosity, string msg, string file, string caller, int line)
        {
            Time = DateTime.Now;
            Verbosity = verbosity;
            Message = msg;
            File = Path.GetFileName(file);
            Caller = caller;
            Line = line;
        }
    }
    static class Logger
    {
        private static int _filter = (int)(Verbosity.Error | Verbosity.Warning | Verbosity.Display);
        private static readonly ObservableCollection<LogMessage> _logMessages = new ObservableCollection<LogMessage>();
        public static ReadOnlyObservableCollection<LogMessage> LogMessages { get; } 
            = new ReadOnlyObservableCollection<LogMessage>(_logMessages);

        public static CollectionViewSource FilteredMessages { get; } = new CollectionViewSource() { Source = LogMessages };

        public static async void Log(Verbosity verbosity, string msg,
            [CallerFilePath] string file = "", [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                _logMessages.Add(new LogMessage(verbosity, msg, file, caller, line));
            }));
        }

        public static async void Clear()
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                _logMessages.Clear();
            }));
        }

        public static void SetFilter(int mask)
        {
            _filter = mask;
            FilteredMessages.View.Refresh();
        }

        static Logger()
        {
            // メッセージをフィルターする設定
            FilteredMessages.Filter += (s, e) =>
            {
                var verbosity = (int)(e.Item as LogMessage).Verbosity;
                e.Accepted = (verbosity & _filter) != 0;
            };
        }
    }
}
