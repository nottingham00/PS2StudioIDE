#pragma once
#include <QPlainTextEdit>
#include <QSet>
#include <QHash>

class QCompleter;
class QSyntaxHighlighter;
class LineNumberArea;

class CodeEditor final : public QPlainTextEdit {
public:
    explicit CodeEditor(QWidget* parent=nullptr);
    bool loadFile(const QString& path, QString* error=nullptr);
    bool saveFile(QString* error=nullptr);
    bool saveFileAs(const QString& path, QString* error=nullptr);
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString& p);
    void gotoLine(int line, int column=1);
    int currentLine() const;
    bool toggleBreakpoint(int line=-1);
    bool hasBreakpoint(int line) const { return m_breakpoints.contains(line); }
    const QSet<int>& breakpoints() const { return m_breakpoints; }
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void toggleBreakpointAtY(int y);
    void setCompletionItems(const QStringList& items);
    void setDiagnosticLines(const QHash<int,int>& severities);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void updateLineNumberAreaWidth(int);
    void updateLineNumberArea(const QRect&, int);
    void highlightCurrentLine();
    void showCompletion();
    QString wordUnderCursor() const;
    QWidget* m_lineNumberArea{};
    QString m_filePath;
    QSet<int> m_breakpoints;
    QCompleter* m_completer{};
    QSyntaxHighlighter* m_highlighter{};
    QHash<int,int> m_diagnostics;
};

class LineNumberArea final : public QWidget {
public:
    explicit LineNumberArea(CodeEditor* editor):QWidget(editor),m_editor(editor){}
    QSize sizeHint() const override { return QSize(m_editor->lineNumberAreaWidth(),0); }
protected:
    void paintEvent(QPaintEvent* event) override { m_editor->lineNumberAreaPaintEvent(event); }
    void mousePressEvent(QMouseEvent* event) override;
private:
    CodeEditor* m_editor;
};
