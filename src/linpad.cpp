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
#include "editor/numberededitor.h"
#include "editor/editor.h"
#include "linpad.h"
#include "replaceDialog.h"
#include "findDialog.h"

LinPad::LinPad()
{
    QCoreApplication::setOrganizationName("LinPad");
    QCoreApplication::setApplicationName("LinPad");
    findDialog = 0;
    replaceDialog = 0;
    globalLineNums = true;
    lastDir = ".";

    workspace = new QMdiArea;
    setCentralWidget(workspace);

    connect(workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateMenus()));
    connect(workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateStatusBar()));

    setAcceptDrops(true);
    setLineWrap(false);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    readSettings();

    setWindowTitle(tr("LinPad"));
    setWindowIcon(QPixmap(":/images/appicon.png"));
}

LinPad::~LinPad()
{
    delete statusText;
    delete fileText;

    delete fileMenu;
    delete editMenu;
    delete bookmarksMenu;
    delete windowMenu;
    delete helpMenu;
    delete fileToolBar;
    delete editToolBar;
    delete windowToolBar;
    delete windowActionGroup;

    //File menu
    delete newAction;
    delete openAction;
    delete saveAction;
    delete saveAsAction;
    delete saveAllAction;
    delete revertAction;
    delete printAction;
    delete exitAction;
    //Edit Menu
    delete undoAction;
    delete redoAction;
    delete cutAction;
    delete copyAction;
    delete pasteAction;
    delete deleteAction;
    delete findAction;
    delete replaceAction;
    delete goToAction;
    delete selectAllAction;
    delete insertTimeAction;
    delete lineWrapAction;
    delete lineNumsAction;
    //Bookmarks menu
    delete addBookmarkAction;
    delete removeAllAction;
    //Window Menu
    delete closeAction;
    delete closeAllAction;
    delete tileAction;
    delete cascadeAction;
    delete nextAction;
    delete previousAction;
    //Help Menu
    delete aboutAction;
    delete aboutQtAction;
    delete workspace;
}

void LinPad::newFile()
{
    NumberedEditor *editor = createEditor();
    editor->editor()->newFile();
    editor->show();
}

void LinPad::openFile(const QStringList &fileName)
{
    foreach(QString file, fileName) {
        if(file != "") {
            NumberedEditor *editor = createEditor();
            if (editor->editor()->openFile(file)) {
                editor->show();
            } else {
                editor->close();
            }
        }
        recentFiles.removeAll(file);
        recentFiles.prepend(file);
        updateRecentFileActions();
    }
}

void LinPad::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2").arg(j + 1).arg(QFileInfo(recentFiles[j]).fileName());
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        } else {
            recentFileActions[j]->setVisible(false);
        }
    }
    separatorAction->setVisible(!recentFiles.isEmpty());
}

void LinPad::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
        openFile(QStringList() << action->data().toString());
}

void LinPad::jumpToBookmark()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action && activeEditor())
    {
        activeEditor()->moveCursor(QTextCursor::Start);
        int num = action->data().toInt();
        for(int i = 1; i < num; i++) {
            activeEditor()->moveCursor(QTextCursor::Down);
        }
    }
}

void LinPad::closeEvent(QCloseEvent *event)
{
    workspace->closeAllSubWindows();
    if (activeEditor()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void LinPad::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void LinPad::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;

    QStringList files;
    foreach(QUrl file, urls) {
        QString str = file.toLocalFile();
        if(!str.isEmpty())
            files << str;
    }
    if(!files.isEmpty())
        openFile(files);
}

void LinPad::open()
{
    QString fileFilters = tr("All files (*)\n" "Text files (*.txt)\n" "C++ source (*.cpp)\n" "C source (*.c)\n" "C style header (*.h)");
    QStringList fileName = QFileDialog::getOpenFileNames(this, tr("Open"), lastDir, fileFilters);
    if (fileName.isEmpty())
        return;
    openFile(fileName);
    lastDir = QFileInfo(fileName.at(fileName.size()-1)).absolutePath();
}

void LinPad::save()
{
    if (activeEditor())
        activeEditor()->save();
}

void LinPad::saveAll()
{
    QList<QMdiSubWindow*> allWindows = workspace->subWindowList();
    foreach(QMdiSubWindow* window, allWindows) {
        NumberedEditor *tmp = qobject_cast<NumberedEditor *>(window->widget());
        Editor* doc = tmp->editor();
        if(doc) {
            doc->save();
        }
    }
}

void LinPad::clearBookmarks()
{
    if(activeNumberBar())
        activeNumberBar()->removeAllBookmarks();
}

void LinPad::addBookmark()
{
    if(activeNumberBar() && activeEditor())
        activeNumberBar()->bookmark(activeEditor()->textCursor().blockNumber()+1);
}

void LinPad::saveAs()
{
    if (activeEditor())
        activeEditor()->saveAs();
}

void LinPad::cut()
{
    if (activeEditor())
        activeEditor()->cut();
}

void LinPad::copy()
{
    if (activeEditor())
        activeEditor()->copy();
}

void LinPad::paste()
{
    if (activeEditor())
        activeEditor()->paste();
}

void LinPad::selectAll()
{
    if (activeEditor())
        activeEditor()->selectAll();
}

void LinPad::insertTime()
{
    if (activeEditor())
        activeEditor()->insertTime();
}

void LinPad::print()
{
    if (activeEditor())
        activeEditor()->print();
}

void LinPad::revert()
{
    if (activeEditor()) {
        activeEditor()->revertToInitial();
    }
}

void LinPad::about()
{
    QMessageBox::about(this, tr("About LinPad"),
                       tr("<h1>LinPad v0.2</h1>&copy;2008 Alex Reinking<br>alex.reinking@gmail.com<p>LinPad is a code editor capable of:<ul><li>XML Based <b>Syntax Highlighting</b></li><li><b>Automatic Intentaion</b> for C, C++ and Java</li><li><b>Automatic Completion</b> For C++</li></ul>"));
}

//Nessesary to disable actions that wouldn't do anything without an editor/selection.
void LinPad::updateMenus()
{
    bool hasEditor = (activeEditor() != 0);
    bool hasSelection = activeEditor() && activeEditor()->textCursor().hasSelection();

    saveAction->setEnabled(hasEditor);
    saveAsAction->setEnabled(hasEditor);
    saveAllAction->setEnabled(hasEditor);
    revertAction->setEnabled(hasEditor);
    printAction->setEnabled(hasEditor);

    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    pasteAction->setEnabled(hasEditor);
    deleteAction->setEnabled(hasSelection);
    undoAction->setEnabled(hasEditor);
    redoAction->setEnabled(hasEditor);
    findAction->setEnabled(hasEditor);
    replaceAction->setEnabled(hasEditor);
    goToAction->setEnabled(hasEditor);
    selectAllAction->setEnabled(hasEditor);
    insertTimeAction->setEnabled(hasEditor);
    lineWrapAction->setChecked(globalLineWrap);
    lineNumsAction->setChecked(globalLineNums);

    closeAction->setEnabled(hasEditor);
    closeAllAction->setEnabled(hasEditor);
    tileAction->setEnabled(hasEditor);
    cascadeAction->setEnabled(hasEditor);
    nextAction->setEnabled(hasEditor);
    previousAction->setEnabled(hasEditor);
    windowSeparatorAction->setVisible(hasEditor);

    updateBookmarks();

    if (activeEditor())
        activeEditor()->windowMenuAction()->setChecked(true);
}

void LinPad::updateBookmarks()
{
    if(activeNumberBar() && activeEditor()) {
        foreach(QAction* action, bookmarksMenu->actions())
        {
            if(action != removeAllAction && action != addBookmarkAction)
                bookmarksMenu->removeAction(action);
        }
        foreach(int i, activeNumberBar()->getBookmarks())
        {
            QAction *temp = new QAction( QString(QString::number(i) + ": " + activeEditor()->textAtLine(i).trimmed()) , this);
            bookmarksMenu->addAction(temp);
            temp->setData(i);
            connect(temp, SIGNAL(triggered()), this, SLOT(jumpToBookmark()));
        }
    }
}

//builds the actions
void LinPad::createActions()
{
    newAction = new QAction(QIcon(":/images/filenew.png"), tr("&New"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create a new file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(QIcon(":/images/fileopen.png"), tr("&Open..."),  this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    //Recent Files! Huzzah!
    for(int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    saveAction = new QAction(QIcon(":/images/filesave.png"), tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save the file to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(QIcon(":/images/filesaveas.png"), tr("Save &As..."), this);
    saveAsAction->setShortcut(tr("Ctrl+Shift+S"));
    saveAsAction->setStatusTip(tr("Save the file under a new name"));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveAllAction = new QAction(QIcon(":/images/filesaveall.png"), tr("Save A&ll..."), this);
    saveAllAction->setStatusTip(tr("Save all open files"));
    connect(saveAllAction, SIGNAL(triggered()), this, SLOT(saveAll()));

    revertAction = new QAction(QIcon(":/images/revert.png"), tr("&Revert"), this);
    revertAction->setStatusTip(tr("Revert All Changes"));
    connect(revertAction, SIGNAL(triggered()), this, SLOT(revert()));

    printAction = new QAction(QIcon(":/images/fileprint.png"), tr("&Print"), this);
    printAction->setShortcut(tr("Ctrl+P"));
    printAction->setStatusTip(tr("Print the current document..."));
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

    exitAction = new QAction(QIcon(":/images/exit.png"), tr("&Exit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit LinPad"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    //Edit Menu

    undoAction = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
    undoAction->setShortcut(tr("Ctrl+Z"));
    undoAction->setStatusTip(tr("Undo a mistake"));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));

    redoAction = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
    redoAction->setShortcut(tr("Ctrl+Y"));
    redoAction->setStatusTip(tr("Redo an action"));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));

    cutAction = new QAction(QIcon(":/images/editcut.png"), tr("C&ut"), this);
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut the current selection to the clipboard"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(QIcon(":/images/editcopy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy the current selection to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(QIcon(":/images/editpaste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste the clipboard's contents at the cursor position"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    deleteAction = new QAction(QIcon(":/images/editdelete.png"), tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Deletes the current selection."));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteText()));

    findAction = new QAction(QIcon(":/images/find.png"), tr("&Find"), this);
    findAction->setShortcut(tr("Ctrl+F"));
    findAction->setStatusTip(tr("Finds text in the current document."));
    connect(findAction, SIGNAL(triggered()), this, SLOT(find()));

    replaceAction = new QAction(tr("&Replace"), this);
    replaceAction->setShortcut(tr("Ctrl+R"));
    replaceAction->setStatusTip(tr("Finds text in the current document and replaces it with another string of text."));
    connect(replaceAction, SIGNAL(triggered()), this, SLOT(replace()));

    goToAction = new QAction(tr("&Go To Line"), this);
    goToAction->setShortcut(tr("Ctrl+G"));
    goToAction->setStatusTip(tr("Automatically points the cursor to the line number asked for."));
    connect(goToAction, SIGNAL(triggered()), this, SLOT(goToLine()));

    selectAllAction = new QAction(tr("&Select All"), this);
    selectAllAction->setShortcut(tr("Ctrl+A"));
    selectAllAction->setStatusTip(tr("Selects all of the text in the document."));
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));

    insertTimeAction = new QAction(QIcon(":/images/ktimer.png"), tr("&Time/Date"), this);
    insertTimeAction->setShortcut(tr("F5"));
    insertTimeAction->setStatusTip(tr("Inserts the current time and date at the cursor's position."));
    connect(insertTimeAction, SIGNAL(triggered()), this, SLOT(insertTime()));

    lineWrapAction = new QAction(tr("&Line Wrap"), this);
    lineWrapAction->setCheckable(true);
    lineWrapAction->setChecked(globalLineWrap);
    lineWrapAction->setStatusTip(tr("Toggles The Document's Line Wrap Functionality."));
    connect(lineWrapAction, SIGNAL(toggled(bool)), this, SLOT(setLineWrap(bool)));

    lineNumsAction = new QAction(tr("Line &Numbers"), this);
    lineNumsAction->setShortcut(tr("F6"));
    lineNumsAction->setCheckable(true);
    lineNumsAction->setChecked(globalLineNums);
    lineNumsAction->setStatusTip(tr("Toggles The Document's Line Number Visibility."));
    connect(lineNumsAction, SIGNAL(toggled(bool)), this, SLOT(setLineNums(bool)));

    //Bookmarks Menu
    removeAllAction = new QAction(tr("&Remove all Bookmarks"), this);
    removeAllAction->setStatusTip(tr("Removes All Bookmarks in the List"));
    connect(removeAllAction, SIGNAL(triggered()), this, SLOT(clearBookmarks()));

    addBookmarkAction = new QAction(tr("&Add bookmark"), this);
    addBookmarkAction->setShortcut(tr("Ctrl+B"));
    addBookmarkAction->setStatusTip(tr("Adds A Bookmark at the Current Line"));
    connect(addBookmarkAction, SIGNAL(triggered()), this, SLOT(addBookmark()));

    //Window Menu

    closeAction = new QAction(QIcon(":/images/fileclose.png"), tr("Cl&ose"), this);
    closeAction->setShortcut(tr("Ctrl+F4"));
    closeAction->setStatusTip(tr("Close the active window"));
    connect(closeAction, SIGNAL(triggered()), workspace, SLOT(closeActiveSubWindow()));

    closeAllAction = new QAction(tr("Close &All"), this);
    closeAllAction->setShortcut(tr("Ctrl+Shift+F4"));
    closeAllAction->setStatusTip(tr("Close all the windows"));
    connect(closeAllAction, SIGNAL(triggered()), workspace, SLOT(closeAllSubWindows()));

    tileAction = new QAction(tr("&Tile"), this);
    tileAction->setStatusTip(tr("Tile the windows"));
    connect(tileAction, SIGNAL(triggered()), workspace, SLOT(tileSubWindows()));

    cascadeAction = new QAction(tr("&Cascade"), this);
    cascadeAction->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAction, SIGNAL(triggered()), workspace, SLOT(cascadeSubWindows()));

    nextAction = new QAction(QIcon(":/images/next.png"), tr("Ne&xt"), this);
    nextAction->setShortcut(tr("Ctrl+F6"));
    nextAction->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAction, SIGNAL(triggered()), workspace, SLOT(activateNextSubWindow()));

    previousAction = new QAction(QIcon(":/images/previous.png"), tr("Pre&vious"), this);
    previousAction->setShortcut(tr("Ctrl+Shift+F6"));
    previousAction->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAction, SIGNAL(triggered()), workspace, SLOT(activatePreviousSubWindow()));

    windowSeparatorAction = new QAction(this);
    windowSeparatorAction->setSeparator(true);

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setShortcut(tr("F1"));
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setShortcut(tr("Ctrl+F1"));
    aboutQtAction->setStatusTip(tr("Show the Qt Toolkit's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    windowActionGroup = new QActionGroup(this);
}

//Sets up the main menus
void LinPad::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addSeparator();
    fileMenu->addAction(openAction);
    separatorAction = fileMenu->addSeparator();
    for(int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addAction(saveAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction(revertAction);
    fileMenu->addSeparator();
    fileMenu->addAction(printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addAction(deleteAction);
    editMenu->addSeparator();
    editMenu->addAction(findAction);
    editMenu->addAction(replaceAction);
    editMenu->addAction(goToAction);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAction);
    editMenu->addSeparator();
    editMenu->addAction(insertTimeAction);
    editMenu->addSeparator();
    editMenu->addAction(lineWrapAction);
    editMenu->addAction(lineNumsAction);

    bookmarksMenu = menuBar()->addMenu(tr("&Bookmarks"));
    bookmarksMenu->addAction(removeAllAction);
    bookmarksMenu->addAction(addBookmarkAction);

    windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(closeAction);
    windowMenu->addAction(closeAllAction);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAction);
    windowMenu->addAction(cascadeAction);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAction);
    windowMenu->addAction(previousAction);
    windowMenu->addAction(windowSeparatorAction);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

//The two wimpy tool bars, more actions to be added.
void LinPad::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->addAction(printAction);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addAction(deleteAction);

    windowToolBar = addToolBar(tr("Window"));
    windowToolBar->addAction(closeAction);
    windowToolBar->addAction(nextAction);
    windowToolBar->addAction(previousAction);
}

//sets up the status bar
void LinPad::createStatusBar()
{
    statusText = new QLabel;
    fileText = new QLabel;
    statusBar()->addWidget(statusText);
    statusBar()->addWidget(fileText);
    updateStatusBar();
}

//Updates the status bar with the current row/col of the active editor and any selection.
void LinPad::updateStatusBar()
{
    if(!activeEditor()) {
        statusText->setText("");
        fileText->setText("");
        statusText->setVisible(false);
        fileText->setVisible(false);
        return;
    }
    else
    {
        statusText->setVisible(true);
        fileText->setVisible(true);
    }

    QTextCursor c = activeEditor()->textCursor();
    int rows = c.blockNumber() + 1;
    int cols = c.columnNumber() + 1;
    QString text = tr("Row: %1     Col:%2").arg(rows).arg(cols);
    if ( c.hasSelection() ) {
        int region = c.selectionEnd() - c.selectionStart();
        text += tr("     Selected %1 Characters").arg(region);
    }
    statusText->setText(text);

    //For current file.
    fileText->setText(tr("File: %1").arg(activeEditor()->currentFile()));
}

//When there is no Editor open it makes no sence to display information about a closed Editor.
void LinPad::neutralizeInterface()
{
    statusText->setText("");
    foreach(QAction* action, bookmarksMenu->actions())
    {
        if(action != removeAllAction && action != addBookmarkAction)
            bookmarksMenu->removeAction(action);
    }
}

NumberedEditor* LinPad::createEditor()
{
    //Set up NumberedEditor
    NumberedEditor *editor = new NumberedEditor(0);
    editor->editor()->setLineWrap(globalLineWrap);
    editor->setShowing(globalLineNums);

    //Connect internal Editor
    connect(editor->editor(), SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));
    connect(editor->editor(), SIGNAL(copyAvailable(bool)), deleteAction, SLOT(setEnabled(bool)));
    connect(editor->editor(), SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));
    connect(editor->editor(), SIGNAL(cursorPositionChanged()), this, SLOT(updateStatusBar()));
    connect(editor->editor(), SIGNAL(selectionChanged()), this, SLOT(updateStatusBar()));
    connect(editor->editor()->document(), SIGNAL(contentsChanged()), this, SLOT(updateStatusBar()));
    connect(editor->editor(), SIGNAL(closing()), this, SLOT(neutralizeInterface()));
    //This is for the wonder of temporary error messages
    connect(editor->editor(), SIGNAL(dispMsg(QString, int)), this, SLOT(dispMsg(QString, int)));
    //Bookmarks should stay updated...
    connect(editor->numberBar(), SIGNAL(bookmarksChanged()), this, SLOT(updateBookmarks()));
    connect(editor->editor()->document(), SIGNAL(contentsChanged()), this, SLOT(updateBookmarks()));

    //Add it to the interface
    workspace->addSubWindow(editor);
    windowMenu->addAction(editor->editor()->windowMenuAction());
    windowActionGroup->addAction(editor->editor()->windowMenuAction());

    //For those functions who need the NumberedEditor we just made.
    return editor;
}

Editor* LinPad::activeEditor()
{
    if(!workspace)
        return 0;
    if(!workspace->activeSubWindow())
        return 0;
    NumberedEditor *e = qobject_cast<NumberedEditor *>(workspace->activeSubWindow()->widget());
    if(e)
        return e->editor();
    else
        return 0;
}

NumberBar* LinPad::activeNumberBar()
{
    if(!workspace)
        return 0;
    if(!workspace->activeSubWindow())
        return 0;
    NumberedEditor *e = qobject_cast<NumberedEditor *>(workspace->activeSubWindow()->widget());
    if(e)
        return e->numberBar();
    else
        return 0;
}

void LinPad::deleteText()
{
    if(activeEditor())
        activeEditor()->textCursor().removeSelectedText();
}

void LinPad::undo()
{
    if(!activeEditor())
        return;

    activeEditor()->undo();
}

void LinPad::redo()
{
    if(!activeEditor())
        return;

    activeEditor()->redo();
}
//convenience functions for me.
int LinPad::showNotFoundDialog(const QString &retryText = "")
{
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowTitle(tr("Not Found!"));
    msgBox.setText(retryText);
    int x = (int)( this->x() + (this->width()/2) - (msgBox.width()/4) );
    int y = (int)( this->y() + (this->height()/2) - (msgBox.height()/4) );
    msgBox.move(x,y);
    return msgBox.exec();
}

int LinPad::showReplaceDialog(const QString &replaceStr = "")
{
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowTitle(tr("Replace?"));
    msgBox.setText(tr("Replace this text with \'%1\'").arg(replaceStr));

    int x = (int)( this->x() + (this->width()/2) - (msgBox.width()/4) );
    int y = (int)( this->y() + (this->height()/2) - (msgBox.height()/4) );
    msgBox.move(x,y);
    return msgBox.exec();
}

void LinPad::find()
{
    if(!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(find(const QRegExp &, QTextDocument::FindFlags)),
                this, SLOT(findText(const QRegExp &, QTextDocument::FindFlags)));
    }
    findDialog->show();
    findDialog->activateWindow();
}

void LinPad::replace()
{
    if(!replaceDialog) {
        replaceDialog = new ReplaceDialog(this);
        connect(replaceDialog, SIGNAL(replace(const QRegExp &, const QString &, unsigned int)),
                this, SLOT(replaceText(const QRegExp &, const QString &, unsigned int)));
    }
    replaceDialog->show();
    replaceDialog->activateWindow();
}

void LinPad::findText(const QRegExp &text, QTextDocument::FindFlags opts)
{
    if (!activeEditor())
        return;

    activeEditor()->textCursor().clearSelection();
    QString retryText = tr("Text not found from current point.\nWould you like to try again from the beggining?");
    if ( opts.testFlag(QTextDocument::FindBackward) ) {
        activeEditor()->moveCursor(QTextCursor::StartOfWord);
        retryText = tr("Text not found from current point.\nWould you like to try again from the end?");
    }

    QTextCursor found = activeEditor()->document()->find(text,activeEditor()->textCursor().position(),opts);
    if (found == QTextCursor()) { //Not found
        if ( opts.testFlag(QTextDocument::FindBackward) ) {
            activeEditor()->moveCursor(QTextCursor::End);
        } else {
            activeEditor()->moveCursor(QTextCursor::Start);
        }
        return;
    } else {
        activeEditor()->setTextCursor(found);
    }
}

void LinPad::replaceText(const QRegExp &text, const QString &replace, unsigned int opts)
{
    if(!activeEditor())
        return;

    activeEditor()->textCursor().clearSelection();

    QString retryText = tr("Text not found From current point.\nWould you like to try again from the beggining?");

    QTextDocument::FindFlags flags = 0;
    bool replaceAll = false;
    if ( opts &  0x00001 ) {
        activeEditor()->moveCursor(QTextCursor::StartOfWord);
        retryText = tr("Text not found From current point.\nWould you like to try again from the end?");
        flags |= QTextDocument::FindBackward;
    }
    if ( (opts &  0x00002) >> 1 ) {
        replaceAll = true;
    }
    QTextCursor found = activeEditor()->document()->find(text,activeEditor()->textCursor().position(),flags);

    if(!replaceAll) {
        if (found == QTextCursor()) {
            int choise = showNotFoundDialog(retryText);
            if(choise == QMessageBox::Yes) {
                if ( flags.testFlag(QTextDocument::FindBackward) ) {
                    activeEditor()->moveCursor(QTextCursor::End);
                } else {
                    activeEditor()->moveCursor(QTextCursor::Start);
                }
                replaceText(text,replace,opts);
            }
        } else {
            activeEditor()->setTextCursor(found);
            int choise = showReplaceDialog(replace);
            if( choise == QMessageBox::Yes ) {
                activeEditor()->textCursor().removeSelectedText();
                activeEditor()->insertPlainText(replace);
                replaceText(text,replace,opts);
            }
        }
    }
    else
    {
        activeEditor()->moveCursor(QTextCursor::Start);
        found = activeEditor()->document()->find(text,activeEditor()->textCursor().position(),flags);
        while(found != QTextCursor())
        {
            activeEditor()->setTextCursor(found);
            activeEditor()->textCursor().removeSelectedText();
            activeEditor()->insertPlainText(replace);
            found = activeEditor()->document()->find(text,activeEditor()->textCursor().position(),flags);
        }
    }
}

void LinPad::goToLine()
{
    if(!activeEditor())
        return;

    int num = QInputDialog::getInt(this,tr("Go To Line"),tr("Line Number: "),1,1,activeEditor()->document()->blockCount());
    QTextBlock block = activeEditor()->document()->begin();
    QTextCursor cur = activeEditor()->textCursor();
    cur.setPosition(block.position());
    while (block.isValid() && cur.blockNumber() + 1 < num) {
        cur.setPosition(block.position());
        block = block.next();
    }
    activeEditor()->setTextCursor(cur);
}

void LinPad::setLineWrap(bool t)
{
    globalLineWrap = t;
    QList<QMdiSubWindow*> allWindows = workspace->subWindowList();
    foreach(QMdiSubWindow* window, allWindows) {
        NumberedEditor *tmp = qobject_cast<NumberedEditor *>(window->widget());
        Editor* doc = tmp->editor();
        if(doc) {
            doc->setLineWrap(t);
        }
    }
}

void LinPad::setLineNums(bool t)
{
    globalLineNums = t;
    QList<QMdiSubWindow*> allWindows = workspace->subWindowList();
    foreach(QMdiSubWindow* window, allWindows) {
        NumberedEditor *tmp = qobject_cast<NumberedEditor *>(window->widget());
        if(tmp) {
            tmp->setShowing(t);
        }
    }
}

void LinPad::writeSettings()
{
    QSettings settings;

    settings.setValue("maximized", isMaximized());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("wordWrap", globalLineWrap);
    settings.setValue("lineNums", globalLineNums);
    settings.setValue("lastDir", lastDir);

    QStringList writtenRecentFiles;
    int j = 0;
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        writtenRecentFiles << i.next();
        j++;
        if(j >= MaxRecentFiles)
            break;
    }
    settings.setValue("recentFiles", writtenRecentFiles);
}

void LinPad::readSettings()
{
    QSettings settings;

    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(750, 500)).toSize();
    bool max = settings.value("maximized", false).toBool();
    bool wrap = settings.value("wordWrap", false).toBool();
    bool nums = settings.value("lineNums", true).toBool();
    lastDir = settings.value("lastDir", ".").toString();
    recentFiles = settings.value("recentFiles", "").toStringList();
    updateRecentFileActions();

    if(!max) {
        resize(size);
        move(pos);
    } else {
        showMaximized();
    }
    setLineWrap(wrap);
    setLineNums(nums);
}

void LinPad::dispMsg(QString msg, int time)
{
    statusBar()->showMessage(msg, time);
}
