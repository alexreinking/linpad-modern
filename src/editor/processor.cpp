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
#include "processor.h"
#include "editor.h"

/******************************************
* I would like it to be known that        *
* my ONLY design goal for this { bracket- *
* style auto-indenter was simplicity      *
* of comprehension, it *IS* a bit         *
* slower than I would like, but whatever. *
*******************************************/

TextProcessor::TextProcessor(QTextEdit* ed) : QObject(), edit(ed)
{
    autoInsertion = false;
    lastRow = -1;
    preLastRow = -1;
    depth = 0;
    off = true;
    edit->installEventFilter(this);
}

TextProcessor::~TextProcessor()
{
    edit->removeEventFilter(this);
}

bool TextProcessor::eventFilter(QObject *target, QEvent *event)
{
    if(!edit)
        return false;
    if(qobject_cast<Editor *>(edit)->completerShowing()) //just to make sure that the auto-completer isn't open
        return QObject::eventFilter(target,event);

    if(target == edit && !off) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return ) {
                if(off || autoInsertion)
                    return QObject::eventFilter(target,event);

                QTextCursor c = edit->textCursor();
                c.insertText("\n");
                if (c.blockNumber() != 0) {
                    QTextBlock bl = c.block();
                    int tabs = getIndentForLine(c.blockNumber());
                    QString lineBegin = "";
                    int i = 0;
                    while(i < tabs) {
                        lineBegin += "     ";
                        i++;
                    }
                    autoInsertion = true;
                    while (bl.text().startsWith('\t') || bl.text().startsWith(' ')) {
                        c.deleteChar();
                    }
                    c.insertText(lineBegin);
                    edit->setTextCursor(c);
                    autoInsertion = false;
                    return true;
                }
            }

            if(keyEvent->key() == '}' || keyEvent->key() == '{') {
                QTextCursor c = edit->textCursor();
                QTextBlock bl = c.block();
                QString lineBegin = "";
                if (keyEvent->key() == '}')
                    c.insertText("}");
                else if(keyEvent->key() == '{')
                    c.insertText("{");

                int tabs = getIndentForLine(c.blockNumber());
                int i = 0;
                while(i < tabs) {
                    lineBegin += "     ";
                    i++;
                }
                autoInsertion = true;
                c.movePosition(QTextCursor::StartOfLine);
                while (bl.text().startsWith('\t') || bl.text().startsWith(' ')) {
                    c.deleteChar();
                }
                c.insertText(lineBegin);
                c.movePosition(QTextCursor::EndOfLine);
                edit->setTextCursor(c);
                autoInsertion = false;
                return true;
            }
        }
    }
    return QObject::eventFilter(target,event);
}

int TextProcessor::getIndentForLine(int lineNum)
{
    QStringList lines = edit->toPlainText().split('\n');
    int i, indent = 0;
    bool inMultiLineComment = false;
    for(i = 0; i < lineNum; ++i) {
        QString line = lines.at(i).trimmed();
        QString prevLine;
        if(i==0)
            prevLine = "";
        else
            prevLine = lines.at(i-1).trimmed();

        if(mCommentStart(line) != -1)
            line.chop(line.size() - mCommentStart(line));
        if(mCommentEnd(line) != -1)
            line.remove(0,mCommentEnd(line));

        line = clean(line);
        prevLine = clean(prevLine);
        int pos = 0;
        while(pos < line.size()) {
            if(line.at(pos) != '{' && line.at(pos) != '}') {
                line.remove(pos,1);
                pos--;
            }
            pos++;
        }
        pos = 0;
        int change = 0;
        while(pos < line.size()) {
            if(inMultiLineComment)
                break;
            if(line.at(pos) == QChar('{'))
                change++;
            else if(line.at(pos) == QChar('}'))
                change--;
            pos++;
        }
        line = lines.at(i).trimmed();
        if(mCommentStart(line) != -1)
            inMultiLineComment = true;
        if(mCommentEnd(line) != -1)
            inMultiLineComment = false;
        indent += change;
    }
    QString prevToCurrentLine = "";
    QString fullPTCL = "";
    QString currentLine = lines.at(lineNum);
    currentLine = clean(currentLine);
    int pos = 0;
    while(pos < currentLine.size()) {
        if(currentLine.at(pos) != '{' && currentLine.at(pos) != '}') {
            currentLine.remove(pos,1);
            pos--;
        }
        pos++;
    }
    if(lineNum>=1) {
        prevToCurrentLine = lines.at(lineNum-1).trimmed();
        fullPTCL = lines.at(lineNum-1);
    }
    //Strip EVERYTHING from prevToCurrentLine
    prevToCurrentLine = clean(prevToCurrentLine);

    if(((prevToCurrentLine.startsWith("if") || prevToCurrentLine.startsWith("else if") || prevToCurrentLine.startsWith("for")) && prevToCurrentLine.endsWith(")")) && !currentLine.trimmed().startsWith("{")) {
        indent++;
    }
    if(clean(prevToCurrentLine).startsWith("else") && !clean(prevToCurrentLine).startsWith("else if") && !currentLine.trimmed().startsWith("{") && !prevToCurrentLine.endsWith("{") && !prevToCurrentLine.endsWith(";") ) {
        indent++;
    }
    while(currentLine.trimmed().endsWith("}") || currentLine.trimmed().startsWith("}")) { //seems to work. may have bugs since it took me 2 mins to write
        if(!currentLine.startsWith("{")) //added
        { //added
            indent--;
            currentLine.remove(0,1); //added
        } //added
        if(currentLine.trimmed().endsWith("}"))
            currentLine.chop(1);
        else if(currentLine.trimmed().startsWith("}")) //changed from "else"
            currentLine.remove(0,1);
    }
    if(indent < 0) //sanity check ;-)
        indent = 0;
    return indent;
}

