#pragma once
#include <QWidget>
#include <QProcess>
class QPlainTextEdit; class QLineEdit; class QLabel; class QTabWidget; class QListWidget;
class DebuggerWidget final : public QWidget {
public:
    explicit DebuggerWidget(QWidget* parent=nullptr);
    bool startDebugger(const QString& executable,const QString& elf,const QString& workingDir);
    void stopDebugger(); bool isRunning() const; void sendCommand(const QString& command); void toggleBreakpoint(const QString& file,int line,bool enabled);
private:
    void refreshPane(const QString& command,QPlainTextEdit* target); void consumeOutput(const QString& text);
    QProcess* m_process{}; QPlainTextEdit* m_output{}; QPlainTextEdit* m_locals{}; QPlainTextEdit* m_stack{}; QPlainTextEdit* m_registers{}; QPlainTextEdit* m_memory{}; QLineEdit* m_command{}; QLineEdit* m_watchInput{}; QListWidget* m_watches{}; QLabel* m_status{}; QPlainTextEdit* m_capture{};
};
