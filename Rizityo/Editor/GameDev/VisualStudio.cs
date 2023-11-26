using Editor.GameProject;
using Editor.Utility;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Threading;
using System.Windows.Markup;

namespace Editor.GameDev
{
    enum BuildConfigutation
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor
    }

    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progId = "VisualStudio.DTE.17.0";

        private static readonly ManualResetEventSlim _resetEvent = new ManualResetEventSlim(false);
        private static readonly object _lock = new object();

        private static readonly string[] _buildConfigurationNames
            = new string[] { "Debug", "DebugEditor", "Release", "ReleaseEditor" };

        public static string GetConfigurationName(BuildConfigutation config) => _buildConfigurationNames[(int)config];

        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        private static void CallOnSTAThread(Action action)
        {
            Debug.Assert(action != null);
            var thread = new Thread(() =>
            {
                MessageFilter.Register();
                try
                {
                    action();
                }
                catch (Exception ex)
                {
                    Logger.Log(Verbosity.Warning, ex.Message);
                }
                finally
                {
                    MessageFilter.Revoke();
                }
            });

            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            thread.Join();
        }

        private static void OpenVisualStudio_Internal(string solutionPath)
        {
            IRunningObjectTable rot = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;
            try
            {
                if (_vsInstance == null)
                {
                    // 開かれているVisual Stuidioを探す
                    var hResult = GetRunningObjectTable(0, out rot);
                    if (hResult < 0 || rot == null)
                        throw new COMException($"GetRunningObjectTable関数がHRESULT: {hResult:X8}を返しました");

                    rot.EnumRunning(out monikerTable);
                    monikerTable.Reset();

                    hResult = CreateBindCtx(0, out bindCtx);
                    if (hResult < 0 || bindCtx == null)
                        throw new COMException($"CreateBindCtx関数がHRESULT: {hResult:X8}を返しました");

                    IMoniker[] currentMoniker = new IMoniker[1];
                    while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                    {
                        string name = string.Empty;
                        currentMoniker[0]?.GetDisplayName(bindCtx, null, out name);
                        if (name.Contains(_progId))
                        {
                            hResult = rot.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null)
                                throw new COMException($"Running Object TableのGetObject関数がHRESULT: {hResult:X8}を返しました");

                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;

                            var solutionName = string.Empty;
                            CallOnSTAThread(() =>
                            {
                                solutionName = dte.Solution.FullName;
                            });

                            if (solutionName == solutionPath)
                            {
                                _vsInstance = dte;
                                break;
                            }
                        }
                    }

                    // 新規でVisual Studioを作成
                    if (_vsInstance == null)
                    {
                        Type vsType = Type.GetTypeFromProgID(_progId, true);
                        _vsInstance = Activator.CreateInstance(vsType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, "Visual Studioを開けませんでした");
            }
            finally
            {
                if (monikerTable != null)
                    Marshal.ReleaseComObject(monikerTable);
                if (rot != null)
                    Marshal.ReleaseComObject(rot);
                if (bindCtx != null)
                    Marshal.ReleaseComObject(bindCtx);
            }
        }

        public static void OpenVisualStudio(string solutionPath)
        {
            lock (_lock)
            {
                OpenVisualStudio_Internal(solutionPath);
            }
        }

        private static void CloseVisualStudio_Internal()
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance?.Solution.IsOpen == true)
                {
                    _vsInstance.ExecuteCommand("File.SaveAll");
                    _vsInstance.Solution.Close(true);
                }
                _vsInstance?.Quit();
                _vsInstance = null;
            });
        }

        public static void CloseVisualStudio()
        {
            lock (_lock)
            {
                CloseVisualStudio_Internal();
            }
        }

        private static bool AddFilesToSolution_Internal(string solutionFilePath, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio_Internal(solutionFilePath);
            try
            {
                if (_vsInstance != null)
                {
                    CallOnSTAThread(() =>
                    {
                        if (!_vsInstance.Solution.IsOpen)
                            _vsInstance.Solution.Open(solutionFilePath);
                        else
                            _vsInstance.ExecuteCommand("File.SaveAll");

                        foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
                        {
                            if (project.UniqueName.Contains(projectName))
                            {
                                foreach (var file in files)
                                {
                                    project.ProjectItems.AddFromFile(file);
                                }
                            }
                        }

                        var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                        if (!string.IsNullOrEmpty(cpp))
                        {
                            _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                        }
                        _vsInstance.MainWindow.Activate();
                        _vsInstance.MainWindow.Visible = true;
                    });
                }

                return true;

            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debug.Write("Visual Studioにファイルを追加できませんでした");
                return false;
            }
        }

        public static bool AddFilesToSolution(string solutionFilePath, string projectName, string[] files)
        {
            lock (_lock)
            {
                return AddFilesToSolution_Internal(solutionFilePath, projectName, files);
            }
        }

        private static bool IsDebugging_Internal()
        {
            bool result = false;
            CallOnSTAThread(() =>
            {
                result = _vsInstance != null &&
                    (_vsInstance.Debugger.CurrentProgram != null ||
                    _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
            });

            return result;
        }

        public static bool IsDebugging()
        {
            lock (_lock)
            {
                return IsDebugging_Internal();
            }
        }

        private static void OnBulidSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            if (BuildDone)
                return;

            Logger.Log(Verbosity.Display, $"ビルド開始{project}, {projectConfig}, {platform}, {solutionConfig}");
        }

        private static void OnBulidSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool success)
        {
            if (BuildDone)
                return;

            if (success)
                Logger.Log(Verbosity.Display, $"{projectConfig}ビルドは成功しました");
            else
                Logger.Log(Verbosity.Error, $"{projectConfig}ビルドは失敗しました");

            BuildDone = true;
            BuildSucceeded = success;
            _resetEvent.Set();
        }

        private static void BuildSolution_Internal(Project project, BuildConfigutation buildConfigutation, bool showVSWindow = true)
        {
            if (IsDebugging_Internal())
            {
                Logger.Log(Verbosity.Error, "Visual Studioは現在プロセスを実行中です");
                return;
            }

            OpenVisualStudio_Internal(project.SolutionFilePath);
            BuildSucceeded = BuildDone = false;

            CallOnSTAThread(() =>
            {
                _vsInstance.MainWindow.Visible = showVSWindow;
                if (!_vsInstance.Solution.IsOpen)
                    _vsInstance.Solution.Open(project.SolutionFilePath);

                _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBulidSolutionBegin;
                _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBulidSolutionDone;
            });

            var configName = GetConfigurationName(buildConfigutation);
            try
            {
                // 参照しているpdbファイルは削除しようとすると例外を投げるので消されない
                foreach (var pdbFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{configName}"), "*.pdb"))
                {
                    File.Delete(pdbFile);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            CallOnSTAThread(() =>
            {
                _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                _vsInstance.ExecuteCommand("Build.BuildSolution");
                _resetEvent.Wait();
                _resetEvent.Reset();
            });


        }

        public static void BuildSolution(Project project, BuildConfigutation buildConfigutation, bool showVSWindow = true)
        {
            lock (_lock)
            {
                BuildSolution_Internal(project, buildConfigutation, showVSWindow);
            }
        }

        private static void Run_Internal(Project project, BuildConfigutation buildConfigutation, bool debug)
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance != null && !IsDebugging_Internal() && BuildSucceeded)
                {
                    _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
                }
            });
        }

        public static void Run(Project project, BuildConfigutation buildConfigutation, bool debug)
        {
            lock (_lock)
            {
                Run_Internal(project, buildConfigutation, debug);
            }
        }

        private static void Stop_Internal()
        {
            CallOnSTAThread(() =>
            {
                if (_vsInstance != null && IsDebugging_Internal())
                {
                    _vsInstance.ExecuteCommand("Debug.StopDebugging");
                }
            });
        }

        public static void Stop()
        {
            lock (_lock)
            {
                Stop_Internal();
            }
        }
    }

    [ComImport(), Guid("00000016-0000-0000-C000-000000000046"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IOleMessageFilter
    {

        [PreserveSig]
        int HandleInComingCall(int dwCallType, IntPtr hTaskCaller, int dwTickCount, IntPtr lpInterfaceInfo);


        [PreserveSig]
        int RetryRejectedCall(IntPtr hTaskCallee, int dwTickCount, int dwRejectType);


        [PreserveSig]
        int MessagePending(IntPtr hTaskCallee, int dwTickCount, int dwPendingType);
    }

    public class MessageFilter : IOleMessageFilter
    {
        private const int SERVERCALL_ISHANDLED = 0;
        private const int PENDINGMSG_WAITDEFPROCESS = 2;
        private const int SERVERCALL_RETRYLATER = 2;

        // implement IOleMessageFilter interface. 
        [DllImport("Ole32.dll")]
        private static extern int CoRegisterMessageFilter(IOleMessageFilter newFilter, out IOleMessageFilter oldFilter);

        public static void Register()
        {
            IOleMessageFilter newFilter = new MessageFilter();
            int hr = CoRegisterMessageFilter(newFilter, out var oldFilter);
            Debug.Assert(hr >= 0, "COM IMessageFileterの登録に失敗しました");
        }

        public static void Revoke()
        {
            int hr = CoRegisterMessageFilter(null, out var oldFilter);
            Debug.Assert(hr >= 0, "COM IMessageFileterの登録解除に失敗しました");
        }


        int IOleMessageFilter.HandleInComingCall(int dwCallType, System.IntPtr hTaskCaller, int dwTickCount, System.IntPtr lpInterfaceInfo)
        {
            //returns the flag SERVERCALL_ISHANDLED. 
            return SERVERCALL_ISHANDLED;
        }


        int IOleMessageFilter.RetryRejectedCall(System.IntPtr hTaskCallee, int dwTickCount, int dwRejectType)
        {
            // Thread call was refused, try again. 
            if (dwRejectType == SERVERCALL_RETRYLATER)
            {
                Debug.WriteLine("COM serverは忙しいです。再度EnvDTEインターフェースにコールしてください");
                // retry thread call at once, if return value >=0 & <100. 
                return 500;
            }
            return -1;
        }


        int IOleMessageFilter.MessagePending(System.IntPtr hTaskCallee, int dwTickCount, int dwPendingType)
        {
            return PENDINGMSG_WAITDEFPROCESS;
        }

    }
}
