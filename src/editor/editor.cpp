/***************************************************************************
 *   Copyright (C) 2008 by Alex Reinking                                   *
 *   alex.reinking@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtWidgets>
#include <QtPrintSupport/QtPrintSupport>
#include <iostream>
#include "editor.h"
#include "highlighter.h"
#include "processor.h"
using std::cout;
using std::endl;

Editor::Editor(QWidget *parent, QWidget *owner) : QTextEdit(parent)
{
     this->owner = owner;
     //My Own Style for editors!
     //Anybody changing this program is kindly asked to leave this alone(as credit to me).
     //But, if you want to change it, please leave in the first 19 lines of this file.
     QFont font;
     font.setFamily("Courier");
     font.setFixedPitch(true);
     font.setPointSize(10);
     setFont(font);

     QPalette p = palette();
     p.setColor(QPalette::Base, QColor(255, 255, 255));
     p.setColor(QPalette::WindowText, QColor(0, 0, 0));
     p.setColor(QPalette::Text, QColor(0, 0, 0));
     p.setColor(QPalette::AlternateBase, QColor(200,200,200));
     p.setColor(QPalette::Highlight, QColor(0,0,220));
     setPalette(p);

     setAcceptRichText(false);
     setLineWrapMode(QTextEdit::NoWrap);

     dirty = false;
     initialState = "";
     syntax = new Highlighter(document());

     c = new QCompleter(this); //the completer
     c->setModel(modelFromPlainText(this->toPlainText())); //Added QAbstractItemModel(...)
     c->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
     c->setCaseSensitivity(Qt::CaseInsensitive);
     setCompleter(c);

     tp = new TextProcessor(this);
     
     action = new QAction(this);
     action->setCheckable(true);
     connect(action, SIGNAL(triggered()), this, SLOT(show()));
     connect(action, SIGNAL(triggered()), this, SLOT(setFocus()));

     isUntitled = true;
     fileFilters = tr("All files (*)\n" "Text files (*.txt)\n" "C++ source (*.cpp)\n" "C source (*.c)\n" "C style header (*.h)");

     connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
     connect(syntax, SIGNAL(indentChanged(bool)), tp, SLOT(setActive(bool)));

     int tabsPerSpace;
     tabsPerSpace = 5;
     setTabStopWidth( tabsPerSpace * QFontMetrics(font).width(' ')); //5 spaces (My personal favorite)

     setAttribute(Qt::WA_DeleteOnClose);
}

Editor::~Editor()
{
     delete tp;
     delete syntax;
     delete c;
     delete action;
}

QString Editor::textAtLine(int i)
{
	QTextBlock block = document()->begin();
     int j = 1;
	while (block.isValid() && j < i ) {
		block = block.next();
          j++;
	}
	return block.text();
}

void Editor::revertToInitial()
{
     int r = QMessageBox::warning(this, tr("LinPad"),
                                  tr("Are you sure you want to reload file %1 from disk?<br>Undo history will be <b>cleared</b>").arg(strippedName(curFile)),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default | QMessageBox::Escape);

     if (r == QMessageBox::Yes) {
          setPlainText(initialState);
     }
}

void Editor::newFile()
{
     static int documentNumber = 1;

     curFile = tr("document%1.txt").arg(documentNumber);
     if(owner)
          owner->setWindowTitle(curFile);
     action->setText(curFile);
     isUntitled = true;
     ++documentNumber;
}

bool Editor::openFile(const QString &fileName)
{
     if (readFile(fileName)) {
          setCurrentFile(fileName);
          return true;
     } else {
          return false;
     }
}

QString Editor::currentFile()
{
     return curFile;
}

bool Editor::save()
{
     if (isUntitled) {
          return saveAs();
     } else {
          return saveFile(curFile);
     }
}

bool Editor::saveAs()
{
     static QString runningCurFile = curFile; //Usability feature.
     QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), runningCurFile, fileFilters);
     if (fileName.isEmpty())
          return false;

     return saveFile(fileName);
}

QSize Editor::sizeHint() const
{
     return QSize(72 * fontMetrics().width('x'), 25 * fontMetrics().lineSpacing()); //72 Chars 25 lines
}

bool Editor::close()
{
     if (okToContinue()) {
          emit closing();
          return true;
     } else
          return false;
}

void Editor::documentWasModified()
{
     if(document()->toPlainText() == initialState) {
          if(owner)
               owner->setWindowTitle(strippedName(curFile));
          dirty = false;
     } else {
          if(owner)
               owner->setWindowTitle(strippedName(curFile) + "*");
          dirty = true;
     }
}

bool Editor::okToContinue()
{
     if (dirty) {
          int r = QMessageBox::warning(this, tr("LinPad"),
                                       tr("File %1 has been modified.\n Do you want to save your changes?").arg(strippedName(curFile)),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No,
                                       QMessageBox::Cancel | QMessageBox::Escape);
          if (r == QMessageBox::Yes) {
               return save();
          } else if (r == QMessageBox::Cancel) {
               return false;
          }
     }
     return true;
}

bool Editor::saveFile(const QString &fileName)
{
     if (writeFile(fileName)) {
          setCurrentFile(fileName);
          return true;
     } else {
          return false;
     }
}

//This function sets the full internal filepath, the proper highlighting and indenting rules and sets the window action accordingly.
void Editor::setCurrentFile(const QString &fileName)
{
     syntax->setSourceType("");
     QString text = this->toPlainText();
     curFile = fileName;
     isUntitled = false;
     action->setText(strippedName(curFile));
     if(owner)
          owner->setWindowTitle(strippedName(curFile)); /*
     if (fileName.endsWith(".cpp") || fileName.endsWith(".h") || fileName.endsWith(".cxx") || fileName.endsWith(".c++") || fileName.endsWith(".cc") || fileName.endsWith(".hh") || fileName.endsWith(".h++") || fileName.endsWith(".hxx") || fileName.endsWith(".hcc") || fileName.endsWith(".moc")) {
          syntax->setSourceType("C++");

          QRegExp isUsingQt("\\bQ[A-Za-z]+\\b");
          int index = text.indexOf(isUsingQt);
          if (index >= 0)
               syntax->setSourceType("C++/Qt");
     } else if(fileName.endsWith(".c")) {
          syntax->setSourceType("C");
     } else if(fileName.endsWith(".java")) {
          syntax->setSourceType("Java");
     } */
     syntax->setSourceType(QString(".") + QFileInfo(fileName).completeSuffix());
     if(syntax->type() != "") {
          syntax->rehighlight();
     }
     if(c)
          c->setModel(modelFromPlainText(this->toPlainText())); //Added QAbstractItemModel(...)
     setCompleter(c);
     dirty = false;
}

bool Editor::readFile(const QString &fileName)
{
     QFile file(fileName);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          QMessageBox::warning(this, tr("LinPad"),
                               tr("Cannot read file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
          return false;
     }

     QTextStream in(&file);
     QApplication::setOverrideCursor(Qt::WaitCursor);
     QString text = in.readAll();
     initialState = text;
     setPlainText(text);
     setCurrentFile(fileName);
     QApplication::restoreOverrideCursor();
     return true;
}

bool Editor::writeFile(const QString &fileName)
{
     QFile file(fileName);
     if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QMessageBox::warning(this, tr("LinPad"),
                               tr("Cannot write file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
          return false;
     }

     QTextStream out(&file);
     QApplication::setOverrideCursor(Qt::WaitCursor);
     QString text = document()->toPlainText();
     out << text;
     initialState = text;
     QApplication::restoreOverrideCursor();
     return true;
}

QString Editor::strippedName(const QString &fullFileName)
{
     return QFileInfo(fullFileName).fileName();
}

void Editor::setLineWrap(bool t)
{
     if(t)
          setLineWrapMode(QTextEdit::WidgetWidth);
     else
          setLineWrapMode(QTextEdit::NoWrap);
}

void Editor::insertTime()
{
     QDateTime cronos;
     cronos.setTime(QTime::currentTime());
     cronos.setDate(QDate::currentDate());
     insertPlainText(cronos.toString());
}

void Editor::print()
{
     QPrinter printer;
     QPrintDialog *dlg = new QPrintDialog(&printer, this);
     dlg->setWindowTitle(tr("Print Document"));
     if (dlg->exec() == QDialog::Accepted) {
          document()->print(&printer);
     }
}

void Editor::keyPressEvent ( QKeyEvent *event )
{
     if(!tp->active()) //Are we working with code?
     {
          QTextEdit::keyPressEvent(event); //If not, just be normal
          return;
     }
     if (c->popup()->isVisible()) {
          switch (event->key()) {
          case Qt::Key_Enter:
          case Qt::Key_Return:
          case Qt::Key_Escape:
          case Qt::Key_Tab:
          case Qt::Key_Backtab:
               event->ignore();
               return;
          default:
               break;
          }
     }

     int pos = textCursor().block().text().indexOf(QRegExp("[^ \t]"));
     bool shift, control;
     if (event->modifiers() & Qt::ControlModifier)
          control = true;
     else
          control = false;
     if (event->modifiers() & Qt::ShiftModifier)
          shift = true;
     else
          shift = false;
     switch(event->key()) {
     case Qt::Key_D:
          if(control && !shift)
          {
               QTextCursor current = textCursor();
               QTextCursor modified = textCursor();
               modified.movePosition(QTextCursor::StartOfLine);
               setTextCursor(modified);
               insertPlainText("// ");
               setTextCursor(current);
          } else if(control && shift)
          {
               QTextCursor current = textCursor();
               QTextCursor modified = textCursor();
               modified.movePosition(QTextCursor::StartOfLine);
               setTextCursor(modified);
               if(current.block().text().startsWith("// "))
               {
                    int i = 0;
                    while(i < 3)
                    {
                         textCursor().deleteChar();
                         i++;
                    }
               }
               setTextCursor(current);
          }
          else
          {
               QTextEdit::keyPressEvent(event);
          }
          break;
     case Qt::Key_Home:
          textCursor().clearSelection();
          if(!control && !shift) {
               QTextCursor tc = textCursor();
               tc.movePosition(QTextCursor::StartOfLine);
               tc.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,pos);
               setTextCursor(tc);
               break;
          }
          if(control && !shift) {
               moveCursor(QTextCursor::Start);
               break;
          }
          if(shift && !control) {
               pos = textCursor().block().text().indexOf(QRegExp("[^ \t]"));
               QTextCursor tc = textCursor();
               tc.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
               tc.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,pos);
               setTextCursor(tc);
               break;
          }
          if(!shift && !control) {
               QTextCursor cursor = textCursor();
               cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
               setTextCursor(cursor);
          }
          break;
     default:
          bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_E); // CTRL+E

          if (!c || !isShortcut) // dont process the shortcut when we have a completer
               QTextEdit::keyPressEvent(event);

          const bool ctrlOrShift = event->modifiers() & (/*Qt::ControlModifier | */ Qt::ShiftModifier);

          static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
          bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
          QString completionPrefix = textUnderCursor();

          if (!isShortcut && (hasModifier || event->text().isEmpty() || completionPrefix.length() < 3 || eow.contains(event->text().right(1)))) {
               c->popup()->hide();
               return;
          }
          if (completionPrefix != c->completionPrefix()) {
               c->setCompletionPrefix(completionPrefix);
               c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
          }
          QRect cr = cursorRect();
          cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());
          c->complete(cr);
     }
}

//Completer Methods
void Editor::setCompleter(QCompleter *completer)
{
     if (c)
          disconnect(c, 0, this, 0);

     c = completer;

     if (!c) {
          c = 0;
          return;
     }
     c->setCompletionMode(QCompleter::PopupCompletion);
     c->setCaseSensitivity(Qt::CaseInsensitive);
     c->setWidget(this);
     connect(c, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
}

QCompleter* Editor::completer() const
{
     return c;
}

void Editor::insertCompletion(const QString& completion)
{
     QTextCursor tc = textCursor();
     int extra = completion.length() - c->completionPrefix().length();
     tc.insertText(completion.right(extra));
     tc.movePosition(QTextCursor::EndOfWord);
     setTextCursor(tc);
}

QString Editor::textUnderCursor() const
{
     QTextCursor tc = textCursor();
     tc.select(QTextCursor::WordUnderCursor);
     return tc.selectedText();
}
bool Editor::completerShowing()
{
     if(c) {
          if(c->popup())
               return c->popup()->isVisible();
          else
               return false;
     }
     else
          return false;
}

QStringListModel *Editor::modelFromPlainText(const QString& data) //Scans C++ file for symbols :-) Works fine, to the extent of my knowledge it doesn't miss-identify symbols it has only ever overlooked one.
{
     QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
     QStringList words = data.split("\n");


     //This will pick up the majority of symbol names, but some completion is MUCH better than no completion.
     //A small "bug" (Could be used to advantage) is that it picks up the names of forwardly-decalred classes too.
     QStringList symbols;

     QRegExp prototypeDef("(?!return)(?!delete)(const|unsigned|signed)?\\w+(\\s+\\*?|\\*?\\s+)(,?\\s?\\*?\\w+\\s?,?\\s?)*;"); //ADDED: (const|unsigned|signed)?
     QRegExp definitionDef("(const|unsigned|signed)?\\s?\\w+(\\s+\\*?|\\*?\\s+)\\w+\\s?=\\s?[A-Za-z0-9+-_()*& ]*\\s?;"); //ADDED: (const|unsigned|signed)?
     QRegExp functionDef("(?!emit)\\w+\\s+\\w+\\((\\s?\\w+(\\s+[\\*&]?|[\\*&]?\\s+)(\\s+\\w+)?(\\s?=\\s?\\w+)?\\s?,?)*\\);");
     QRegExp includeDef("^#include( )+\".+\""); //Added for include parsing
     QRegExpValidator prototypeValidator(prototypeDef, this);
     QRegExpValidator definitionValidator(definitionDef, this);
     QRegExpValidator functionValidator(functionDef, this);
     QRegExpValidator includeValidator(includeDef, this);
     
     QMutableStringListIterator symbolIterator(words);
     int start = 0, lineNo = 1;
     bool inMultiLineComment;
     inMultiLineComment = false;
     while(symbolIterator.hasNext()) {
          QString tmp = symbolIterator.next().simplified();
          if( prototypeValidator.validate(tmp, start) == QValidator::Acceptable ) {
               QRegExp capture("(\\w+)*");
               int pos = capture.indexIn(tmp);
               pos = capture.matchedLength();
               QString restOfStr = tmp.mid(pos+1, tmp.indexOf(';')).simplified(); //space and semicolon
               restOfStr.chop(1);
               QStringList names = restOfStr.split(',');
//                if(restOfStr.split(' ').size() < names.size())
//                     continue;
               foreach(QString str, names) {
                    QString m = str.simplified();
                    if(m.split(QChar(' ')).size() > 1) //sanity check :-)
                         continue;
                    if(!symbols.contains(QByteArray(qPrintable(m)))) {
                         symbols << QByteArray(qPrintable(m));
                    }
               }
          } else if( definitionValidator.validate(tmp, start) == QValidator::Acceptable ) {
               QRegExp capture("(\\w+\\s?=)");
               int pos = capture.indexIn(tmp);
               pos = capture.pos(0);
               QString str = tmp.mid(pos, tmp.indexOf('=') - pos).simplified();
               if(!symbols.contains(QByteArray(qPrintable(str)))) {
                    symbols << QByteArray(qPrintable(str));
               }
          } else if( functionValidator.validate(tmp, start) == QValidator::Acceptable ) {
               QRegExp capture("(\\w+\\()");
               int pos = capture.indexIn(tmp);
               pos = capture.pos(0);
               QString str = tmp.mid(pos, tmp.indexOf('(') - pos).simplified();
               str += '(';
               if(!symbols.contains(QByteArray(qPrintable(str)))) {
                    symbols << QByteArray(qPrintable(str));
               }
          } else if ( includeValidator.validate(tmp, start) == QValidator::Acceptable ) { //START: added1
               QRegExp capture("\".+");
               int pos = capture.indexIn(tmp);
               pos = capture.pos(0);
               QString str = tmp.mid(pos+1,tmp.indexOf('\"', pos+1)-pos-1);
               QFile file(QFileInfo(curFile).absoluteDir().absoluteFilePath(str));
               if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//                     QMessageBox::warning(this, tr("LinPad"), tr("Cannot parse file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
                    //statusBar()->showMessage(tr("Cannot parse file %1:\n%2.").arg(file.fileName()).arg(file.errorString()), 2000);
                    emit dispMsg(tr("Cannot parse file %1:\n%2.").arg(file.fileName()).arg(file.errorString()), 2000);
               } else {
                    QTextStream in(&file);
                    QString text = in.readAll();
                    symbols << modelFromPlainText(text)->stringList();
               }
          } //END: added2
          ++lineNo;
     }
     qSort(symbols);
     QApplication::restoreOverrideCursor();
     return new QStringListModel(symbols, c);
}

void Editor::commentLine()
{
     QTextCursor current = textCursor();
     QTextCursor modified = textCursor();
     modified.movePosition(QTextCursor::StartOfLine);
     setTextCursor(modified);
     insertPlainText("// ");
     setTextCursor(current);
}

void Editor::uncommentLine()
{
     QTextCursor current = textCursor();
     QTextCursor modified = textCursor();
     modified.movePosition(QTextCursor::StartOfLine);
     setTextCursor(modified);
     if(current.block().text().startsWith("// "))
     {
          int i = 0;
          while(i < 3)
          {
               textCursor().deleteChar();
               i++;
          }
     }
     setTextCursor(current);
}

void Editor::contextMenuEvent(QContextMenuEvent *e)
{
     std::cout << "Max val: " << verticalScrollBar()->maximum() << std::endl;
     std::cout << "Min val: " << verticalScrollBar()->minimum() << std::endl;
     std::cout << "Cur val: " << verticalScrollBar()->value() << std::endl;
     QMenu *menu = new QMenu;
     menu->addAction(tr("Undo"), this, SLOT(undo()), QKeySequence(tr("Ctrl+U")));
     menu->addAction(tr("Redo"), this, SLOT(redo()), QKeySequence(tr("Ctrl+Y")));
     menu->addSeparator();
     menu->addAction(tr("Cut"), this, SLOT(cut()), QKeySequence(tr("Ctrl+X")));
     menu->addAction(tr("Copy"), this, SLOT(copy()), QKeySequence(tr("Ctrl+C")));
     menu->addAction(tr("Paste"), this, SLOT(paste()), QKeySequence(tr("Ctrl+V")));
     menu->addAction(tr("Select All"), this, SLOT(selectAll()), QKeySequence(tr("Ctrl+A")));
     if(tp->active())
     {
          menu->addSeparator();
          menu->addAction(tr("Comment Line"), this, SLOT(commentLine()), QKeySequence(tr("Ctrl+D")));
          menu->addAction(tr("Uncomment Line"), this, SLOT(uncommentLine()), QKeySequence(tr("Ctrl+Shift+D")));
     }
     menu->exec(e->globalPos());
     delete menu;
}
