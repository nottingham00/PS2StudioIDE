#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QJsonObject>
#include <QProcessEnvironment>
#include "ClangdClient.h"
class QTreeWidget; class QTabWidget; class QPlainTextEdit; class QLabel; class QComboBox; class QListWidget; class InspectorWidget; class SceneDocument; class SceneView2D; class Viewport3D; class AssetDatabase; class GameView; class ProfilerWidget; class QTimer; class QFileSystemWatcher; class QCloseEvent; class QDockWidget;
class CodeEditor; class DebuggerWidget; class ProblemsWidget;

class MainWindow: public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched,QEvent* event) override;
private slots:
    void newProject(); void openProject(); void saveCurrent(); void saveCurrentAs();
    void newSourceFile(); void newHeaderFile(); void newComponent(); void findInEditor(); void replaceInEditor(); void gotoLine(); void toggleBreakpoint(); void findInProject(); void goToDefinition(); void findReferences(); void renameSymbol(); void showHover(); void requestSmartCompletion(); void restartPCSX2(); void convertModernModel();
    void buildProject(); void buildAllProfiles(); void buildAndRun(); void runPCSX2(); void stopRun(); void sendToPS2(); void openDiscDeploy(); void startDebugger(); void refreshEnvironment();
    void newScene(); void saveScene(); void addSceneObject(); void deleteSceneObject(); void duplicateSceneObject(); void importAsset(); void createPrefab(); void instantiatePrefab();
    void autosave(); void recoverAutosave(); void onWatchedPathChanged(const QString&); void navigateBuildError(); void configureProject(); void openEnvironmentDoctor();
    void openSetupWizard(); void openDependencyManager(); void openDocumentation(); void openValidationCenter(); void checkForUpdates(); void createSupportBundle(); void create2DSample(); void create3DSample(); void createValidationSample();
    void regenerateRuntimeTemplates(); void setLiveReload(bool enabled);
private:
    void buildUi(); void buildMenus(); void applyTheme(); void createProject(const QString&,const QString&,const QString&,const QString& engineBackend="native"); void loadProjectTree(const QString&); void openFile(const QString&); void loadDefaultScene(); void refreshHierarchy(); void refreshAssets(); void selectObject(const QString&); QString findExecutable(const QStringList&)const; void appendOutput(const QString&); void updateEnvironmentPanel(); bool exportAllScenes(QString*error=nullptr); QString findElf()const;
    void loadAppState(); void saveAppState(); void updateFileWatchers(); void rememberProject(const QString&); void writeProjectConfig(); QJsonObject readProjectConfig() const; QString projectMetaDir() const;
    CodeEditor* currentEditor() const; bool saveEditor(CodeEditor*); bool saveAllEditors(); bool closeEditorTab(int index); void updateEditorTab(CodeEditor*); void createCodeFile(const QString& subdir,const QString& suffix);
    QString resolveMake() const; QString resolvePCSX2() const; QProcessEnvironment buildEnvironment() const; bool ensureIopSkeleton(QString* error=nullptr); void prepareRuntimeSupportFiles();
    QString m_projectRoot,m_selectedObjectId; QTreeWidget*m_projectTree{};QTreeWidget*m_hierarchy{};QTabWidget*m_workspaceTabs{};QTabWidget*m_editorTabs{};QPlainTextEdit*m_buildOutput{};QListWidget*m_assets{};QLabel*m_statusTarget{};QLabel*m_envSummary{};QComboBox*m_targetCombo{};QComboBox*m_profileCombo{};QComboBox*m_videoPresetCombo{};QProcess*m_process{};QProcess*m_runProcess{};InspectorWidget*m_inspector{};SceneDocument*m_scene{};SceneView2D*m_scene2D{};Viewport3D*m_view3D{};AssetDatabase*m_assetDb{};GameView*m_gameView{};ProfilerWidget*m_profiler{};QTimer*m_autosaveTimer{};QFileSystemWatcher*m_watcher{};
    DebuggerWidget* m_debugger{}; QDockWidget* m_debugDock{}; ProblemsWidget* m_problems{}; QDockWidget* m_problemsDock{}; QHash<QString,QList<ClangdDiagnostic>> m_diagnostics;
    QString m_pcsx2Path,m_ps2Host="192.168.0.10"; int m_autosaveSeconds=60; bool m_sceneDirty=false; bool m_runAfterBuild=false; bool m_liveReload=false; bool m_liveReloadPending=false; ClangdClient* m_clangd{};
};
