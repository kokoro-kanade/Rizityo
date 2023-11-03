using Editor.GameProject;
using Editor.Utility;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace Editor.GameDev
{
    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progId = "VisualStudio.DTE.16.0";

        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        public static void OpenVisualStudio(string solutionPath)
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
                            var solutionName = dte.Solution.FullName;
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

        public static void CloseVisualStudio()
        {
            if (_vsInstance?.Solution.IsOpen == true)
            {
                _vsInstance.ExecuteCommand("File.SaveAll");
                _vsInstance.Solution.Close(true);
            }
            _vsInstance?.Quit();
        }

        public static bool AddFilesToSolution(string solutionFilePath, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solutionFilePath);
            try
            {
                if(_vsInstance != null)
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

        public static bool IsDebugging()
        {
            bool result = false;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    result = _vsInstance != null &&
                        (_vsInstance.Debugger.CurrentProgram != null ||
                        _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
                    if (result)
                        break;

                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    // おそらくVSが忙しくて対応できていないので待つ
                    if (!result)
                        System.Threading.Thread.Sleep(1000);
                }

            }
            return result;
        }

        private static void OnBulidSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
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
        }

        public static void BuildSolution(Project project, string configName, bool showVSWindow = true)
        {
            if (IsDebugging())
            {
                Logger.Log(Verbosity.Error, "Visual Studioは現在プロセスを実行中です");
                return;
            }

            OpenVisualStudio(project.SolutionFilePath);
            BuildSucceeded = BuildDone = false;

            for (int i = 0; i < 3; i++)
            {
                try
                {
                    if (!_vsInstance.Solution.IsOpen)
                        _vsInstance.Solution.Open(project.SolutionFilePath);
                    _vsInstance.MainWindow.Visible = showVSWindow;

                    _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBulidSolutionBegin;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBulidSolutionDone;

                    try
                    {
                        // 参照しているpbdファイルは削除しようとすると例外を投げるので消されない
                        foreach (var pbdFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{configName}"), "*.pbd"))
                        {
                            File.Delete(pbdFile);
                        }
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine(ex.Message);
                    }

                    _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                    _vsInstance.ExecuteCommand("Build.BuildSolution");
                    
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Debug.WriteLine($"{i}回目: {project.Name}のビルドに失敗");
                    System.Threading.Thread.Sleep(1000);
                }
            }
            
        }

    }
}
