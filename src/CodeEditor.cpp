#include "CodeEditor.h"
#include <QtWidgets>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QPainter>
#include <QTextBlock>
#include <QCompleter>
#include <QStringListModel>
#include <QFileInfo>
#include <QSaveFile>
#include <QKeyEvent>

class CppHighlighter final : public QSyntaxHighlighter {
public:
    explicit CppHighlighter(QTextDocument* doc):QSyntaxHighlighter(doc){
        auto fmt=[&](QColor c,bool bold=false){QTextCharFormat f;f.setForeground(c);if(bold)f.setFontWeight(QFont::Bold);return f;};
        keyword=fmt(QColor("#c792ea"),true); type=fmt(QColor("#82aaff"),true); stringFmt=fmt(QColor("#c3e88d")); commentFmt=fmt(QColor("#637777")); numberFmt=fmt(QColor("#f78c6c")); preproc=fmt(QColor("#ffcb6b")); functionFmt=fmt(QColor("#82aaff"));
        const QStringList kws={"alignas","alignof","asm","auto","break","case","catch","class","const","constexpr","continue","default","delete","do","else","enum","explicit","extern","false","for","friend","goto","if","inline","namespace","new","noexcept","nullptr","operator","private","protected","public","register","reinterpret_cast","return","sizeof","static","static_cast","struct","switch","template","this","throw","true","try","typedef","typename","union","using","virtual","volatile","while"};
        for(const auto& k:kws) rules.push_back({QRegularExpression("\\b"+QRegularExpression::escape(k)+"\\b"),keyword});
        const QStringList types={"bool","char","double","float","int","long","short","signed","unsigned","void","size_t","u8","u16","u32","u64","s8","s16","s32","s64","GSGLOBAL","GSTEXTURE","QWidget","QString"};
        for(const auto& k:types) rules.push_back({QRegularExpression("\\b"+QRegularExpression::escape(k)+"\\b"),type});
        rules.push_back({QRegularExpression(R"("([^"\\]|\\.)*")"),stringFmt});
        rules.push_back({QRegularExpression(R"('([^'\\]|\\.)*')"),stringFmt});
        rules.push_back({QRegularExpression(R"(\b(0x[0-9A-Fa-f]+|\d+(\.\d+)?([eE][+-]?\d+)?[fFuUlL]*)\b)"),numberFmt});
        rules.push_back({QRegularExpression(R"(^\s*#\s*\w+.*$)"),preproc});
        rules.push_back({QRegularExpression(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"),functionFmt});
        rules.push_back({QRegularExpression(R"(//[^\n]*)"),commentFmt});
    }
protected:
    void highlightBlock(const QString& text) override {
        for(const auto&r:rules){auto it=r.re.globalMatch(text);while(it.hasNext()){auto m=it.next();setFormat(m.capturedStart(),m.capturedLength(),r.fmt);}}
        setCurrentBlockState(0);
        int start=0;
        if(previousBlockState()!=1) start=text.indexOf("/*");
        while(start>=0){int end=text.indexOf("*/",start+2);int len;if(end<0){setCurrentBlockState(1);len=text.length()-start;}else len=end-start+2;setFormat(start,len,commentFmt);if(end<0)break;start=text.indexOf("/*",start+len);}
    }
private:
    struct Rule{QRegularExpression re;QTextCharFormat fmt;};
    QVector<Rule> rules; QTextCharFormat keyword,type,stringFmt,commentFmt,numberFmt,preproc,functionFmt;
};

CodeEditor::CodeEditor(QWidget* parent):QPlainTextEdit(parent){
    setLineWrapMode(QPlainTextEdit::NoWrap);setTabStopDistance(fontMetrics().horizontalAdvance(' ')*4.0);
    QFont f("Cascadia Mono");f.setStyleHint(QFont::Monospace);f.setPointSize(10);setFont(f);
    m_lineNumberArea=new LineNumberArea(this);m_highlighter=new CppHighlighter(document());
    connect(this,&QPlainTextEdit::blockCountChanged,this,[this](int n){updateLineNumberAreaWidth(n);});
    connect(this,&QPlainTextEdit::updateRequest,this,[this](const QRect&r,int dy){updateLineNumberArea(r,dy);});
    connect(this,&QPlainTextEdit::cursorPositionChanged,this,[this]{highlightCurrentLine();});
    updateLineNumberAreaWidth(0);highlightCurrentLine();
    QStringList words={"printf","malloc","free","memcpy","memset","gsKit_init_global","gsKit_clear","gsKit_queue_exec","gsKit_sync_flip","gsKit_prim_sprite","gsKit_prim_sprite_texture","gsKit_prim_triangle_3d","gsKit_prim_triangle_texture_3d","padInit","padPortOpen","padRead","SifLoadModule","audsrv_init","audsrv_play_audio","PS2SceneRuntime","PS2Input","PS2Collision","true","false","nullptr","return","if","else","for","while","switch","class","struct","const","static","void","int","float","u32","u64"};
    words.sort(Qt::CaseInsensitive);m_completer=new QCompleter(words,this);m_completer->setWidget(this);m_completer->setCompletionMode(QCompleter::PopupCompletion);m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(m_completer,qOverload<const QString&>(&QCompleter::activated),this,[this](const QString&s){QTextCursor c=textCursor();QString w=wordUnderCursor();c.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor,w.size());c.insertText(s);setTextCursor(c);});
}
void CodeEditor::setFilePath(const QString&p){m_filePath=QFileInfo(p).absoluteFilePath();setProperty("filePath",m_filePath);}
bool CodeEditor::loadFile(const QString&path,QString*error){QFile f(path);if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){if(error)*error=f.errorString();return false;}setPlainText(QString::fromUtf8(f.readAll()));setFilePath(path);document()->setModified(false);return true;}
bool CodeEditor::saveFile(QString*error){if(m_filePath.isEmpty()){if(error)*error="No file path";return false;}QSaveFile f(m_filePath);if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){if(error)*error=f.errorString();return false;}f.write(toPlainText().toUtf8());if(!f.commit()){if(error)*error=f.errorString();return false;}document()->setModified(false);return true;}
bool CodeEditor::saveFileAs(const QString&path,QString*error){setFilePath(path);return saveFile(error);}
int CodeEditor::currentLine()const{return textCursor().blockNumber()+1;}
void CodeEditor::gotoLine(int line,int column){line=qMax(1,line);column=qMax(1,column);QTextBlock b=document()->findBlockByLineNumber(line-1);if(!b.isValid())return;QTextCursor c(b);c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,column-1);setTextCursor(c);centerCursor();setFocus();}
bool CodeEditor::toggleBreakpoint(int line){if(line<1)line=currentLine();if(m_breakpoints.contains(line)){m_breakpoints.remove(line);m_lineNumberArea->update();return false;}m_breakpoints.insert(line);m_lineNumberArea->update();return true;}
int CodeEditor::lineNumberAreaWidth()const{int digits=1,max=qMax(1,blockCount());while(max>=10){max/=10;++digits;}return 20+fontMetrics().horizontalAdvance('9')*digits;}
void CodeEditor::updateLineNumberAreaWidth(int){setViewportMargins(lineNumberAreaWidth(),0,0,0);}
void CodeEditor::updateLineNumberArea(const QRect&r,int dy){if(dy)m_lineNumberArea->scroll(0,dy);else m_lineNumberArea->update(0,r.y(),m_lineNumberArea->width(),r.height());if(r.contains(viewport()->rect()))updateLineNumberAreaWidth(0);}
void CodeEditor::resizeEvent(QResizeEvent*e){QPlainTextEdit::resizeEvent(e);QRect cr=contentsRect();m_lineNumberArea->setGeometry(QRect(cr.left(),cr.top(),lineNumberAreaWidth(),cr.height()));}
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent*event){QPainter p(m_lineNumberArea);p.fillRect(event->rect(),QColor("#0b0f15"));QTextBlock block=firstVisibleBlock();int num=block.blockNumber();int top=(int)blockBoundingGeometry(block).translated(contentOffset()).top();int bottom=top+(int)blockBoundingRect(block).height();while(block.isValid()&&top<=event->rect().bottom()){if(block.isVisible()&&bottom>=event->rect().top()){int line=num+1;p.setPen(QColor("#667085"));p.drawText(16,top,m_lineNumberArea->width()-20,fontMetrics().height(),Qt::AlignRight,QString::number(line));if(m_breakpoints.contains(line)){p.setBrush(QColor("#ff4d5a"));p.setPen(Qt::NoPen);p.drawEllipse(QPoint(8,top+fontMetrics().height()/2),5,5);}}block=block.next();top=bottom;bottom=top+(int)blockBoundingRect(block).height();++num;}}
void CodeEditor::highlightCurrentLine(){QList<QTextEdit::ExtraSelection> sels;if(!isReadOnly()){QTextEdit::ExtraSelection s;s.format.setBackground(QColor("#18202c"));s.format.setProperty(QTextFormat::FullWidthSelection,true);s.cursor=textCursor();s.cursor.clearSelection();sels<<s;}for(auto it=m_diagnostics.cbegin();it!=m_diagnostics.cend();++it){QTextBlock b=document()->findBlockByLineNumber(it.key()-1);if(!b.isValid())continue;QTextEdit::ExtraSelection d;d.cursor=QTextCursor(b);d.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);d.format.setUnderlineColor(it.value()<=1?QColor("#ff5c70"):QColor("#ffc857"));sels<<d;}setExtraSelections(sels);}
QString CodeEditor::wordUnderCursor()const{QTextCursor c=textCursor();c.select(QTextCursor::WordUnderCursor);return c.selectedText();}
void CodeEditor::showCompletion(){QString prefix=wordUnderCursor();m_completer->setCompletionPrefix(prefix);QRect cr=cursorRect();cr.setWidth(m_completer->popup()->sizeHintForColumn(0)+m_completer->popup()->verticalScrollBar()->sizeHint().width());m_completer->complete(cr);}
void CodeEditor::keyPressEvent(QKeyEvent*e){if(m_completer->popup()->isVisible()){switch(e->key()){case Qt::Key_Enter:case Qt::Key_Return:case Qt::Key_Escape:case Qt::Key_Tab:case Qt::Key_Backtab:e->ignore();return;default:break;}}
    if(e->modifiers()==Qt::ControlModifier&&e->key()==Qt::Key_Space){showCompletion();return;}
    if(e->key()==Qt::Key_Tab&&!(e->modifiers()&Qt::ControlModifier)){insertPlainText("    ");return;}
    if(e->key()==Qt::Key_Return||e->key()==Qt::Key_Enter){QTextCursor c=textCursor();QString line=c.block().text();QString indent;for(QChar ch:line){if(ch==' '||ch=='\t')indent+=ch;else break;}QPlainTextEdit::keyPressEvent(e);insertPlainText(indent);return;}
    QPlainTextEdit::keyPressEvent(e);
}
void CodeEditor::toggleBreakpointAtY(int y){QTextBlock block=firstVisibleBlock();int top=(int)blockBoundingGeometry(block).translated(contentOffset()).top();while(block.isValid()){int bottom=top+(int)blockBoundingRect(block).height();if(y>=top&&y<bottom){toggleBreakpoint(block.blockNumber()+1);return;}block=block.next();top=bottom;}}
void LineNumberArea::mousePressEvent(QMouseEvent*event){m_editor->toggleBreakpointAtY((int)event->position().y());}

void CodeEditor::setCompletionItems(const QStringList& items){
    auto* model=qobject_cast<QStringListModel*>(m_completer->model());
    if(!model){model=new QStringListModel(m_completer);m_completer->setModel(model);}
    model->setStringList(items);showCompletion();
}
void CodeEditor::setDiagnosticLines(const QHash<int,int>& severities){m_diagnostics=severities;highlightCurrentLine();}
