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
//QRegExp("[^ \t]")
//int lineBeginIndex = prevText.indexOf(QRegExp("[^ \t]"));
#ifndef EDITOR_H
#define EDITOR_H

#include <QTextEdit>
class Highlighter;
class TextProcessor;
class QCompleter;
class QAbstractItemModel;
class QStringListModel;

class Editor : public QTextEdit
{
    Q_OBJECT
public:
    Editor(QWidget *parent = 0, QWidget *owner = 0);
    ~Editor();

    void newFile();
    void print();
    bool openFile(const QString &fileName);
    bool save();
    bool saveAs();
    QSize sizeHint() const;
    QAction* windowMenuAction() const { return action; }

    QString currentFile();
    QString textAtLine(int i);

    //These functions are called inside of a slot.
    void insertTime();
    void revertToInitial();
    void setLineWrap(bool t);
    bool completerShowing();
    bool close();

signals:
    void closing();
    void dispMsg(QString msg, int time);

private slots:
    void documentWasModified();
    void insertCompletion(const QString &completion);
    void commentLine();
    void uncommentLine();

protected:
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *e);

private:
    QWidget *owner;
    Highlighter *syntax;
    TextProcessor *tp;
    bool okToContinue();
    QString textUnderCursor() const;
    bool saveFile(const QString &fileName);
    void setCompleter(QCompleter *c);
    QCompleter *completer() const;
    void setCurrentFile(const QString &fileName);
    bool readFile(const QString &fileName);
    bool writeFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QStringListModel* modelFromPlainText(const QString& data); //Used to be QAbstractItemModel*

    QString curFile, initialState;
    bool isUntitled, dirty;
    QString fileFilters;
    QAction *action;
    QCompleter *c;
};

#endif
