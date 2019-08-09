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

#ifndef LINPAD_H
#define LINPAD_H

#include <QtWidgets>

class FindDialog;
class ReplaceDialog;
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QToolBar;
class QListWidget;
class NumberedEditor;
class NumberBar;
class Editor;

class LinPad : public QMainWindow
{
     Q_OBJECT

public:
     LinPad();
     ~LinPad();

public slots:
     void newFile();
     void openFile(const QStringList &fileName);

protected:
     void closeEvent(QCloseEvent *event);
     void dragEnterEvent(QDragEnterEvent *event);
     void dropEvent(QDropEvent *event);

private slots:
     void open();
     void save();
     void saveAs();
     void saveAll();
     void revert();
     void print();
     void undo();
     void redo();
     void cut();
     void copy();
     void paste();
     void deleteText();
     void find();
     void replace();
     void goToLine();
     void about();
     void updateMenus();
     void updateBookmarks();
     void findText(const QRegExp &text, QTextDocument::FindFlags opts);
     void replaceText(const QRegExp &text, const QString &replace, unsigned int opts);
     void selectAll();
     void insertTime();
     void setLineWrap(bool t);
     void setLineNums(bool t);
     void updateStatusBar();
     void neutralizeInterface();
     void openRecentFile();
     void jumpToBookmark();
     void clearBookmarks();
     void addBookmark();
     void dispMsg(QString msg, int time);

private:
     enum { MaxRecentFiles = 5 };
     bool globalLineWrap, globalLineNums;
     void createActions();
     void createMenus();
     void createToolBars();
     void createStatusBar();
     void readSettings();
     void writeSettings();
     void updateRecentFileActions();
     int showNotFoundDialog(const QString &retryText);
     int showReplaceDialog(const QString &replaceStr);
     NumberedEditor* createEditor();
     Editor* activeEditor();
     NumberBar* activeNumberBar();

     FindDialog* findDialog;
     ReplaceDialog* replaceDialog;
     QMdiArea *workspace;
     QLabel *statusText;
     QLabel *fileText;
     QString lastDir;

     QMenu *fileMenu;
     QMenu *editMenu;
     QMenu *bookmarksMenu;
     QMenu *windowMenu;
     QMenu *helpMenu;
     QToolBar *fileToolBar;
     QToolBar *editToolBar;
     QToolBar *windowToolBar;
     QActionGroup *windowActionGroup;
     
     //File menu
     QAction *newAction;
     QAction *openAction;
     QAction *recentFileActions[MaxRecentFiles];
     QStringList recentFiles;
     QAction *saveAction;
     QAction *saveAsAction;
     QAction *saveAllAction;
     QAction *revertAction;
     QAction *printAction;
     QAction *exitAction;
     //Edit Menu
     QAction *undoAction;
     QAction *redoAction;
     QAction *cutAction;
     QAction *copyAction;
     QAction *pasteAction;
     QAction *deleteAction;
     QAction *findAction;
     QAction *replaceAction;
     QAction *goToAction;
     QAction *selectAllAction;
     QAction *insertTimeAction;
     QAction *lineWrapAction;
     QAction *lineNumsAction;
     //Bookmarks menu
     QAction *addBookmarkAction;
     QAction *removeAllAction;
     //Window Menu
     QAction *closeAction;
     QAction *closeAllAction;
     QAction *tileAction;
     QAction *cascadeAction;
     QAction *nextAction;
     QAction *previousAction;
     //Global Actions
     QAction *separatorAction;
     QAction *windowSeparatorAction;
     //Help Menu
     QAction *aboutAction;
     QAction *aboutQtAction;
};

#endif
