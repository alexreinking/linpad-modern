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
#ifndef _TEXT_PROCESSOR_H_
#define _TEXT_PROCESSOR_H_

#include <QObject>
class QTextEdit;

class TextProcessor : public QObject
{
     Q_OBJECT
public:
     TextProcessor(QTextEdit*);
     ~TextProcessor();
     bool active() { return !off; }

     static QString clean(const QString &text) //removes all comments and strings from a line of code.
     {
          QString ret;
          ret = text.simplified();
          ret.remove(commentIndex(ret), ret.size());
          int qStart = 0;
          int qLen = 0;
          int ch = 0;
          bool inQuote = false;
          bool inChar = false;
          while(ch < ret.size()) {
               if(ret.at(ch) == QChar('\"') && !inQuote) {
                    qStart = ch++;
                    inQuote = true;
                    continue;
               } else if(ret.at(ch) == QChar('\\') && (inQuote||inChar)) {
                    ch+=2;
                    continue;
               } else if(ret.at(ch) == QChar('\'') && !inChar && !inQuote) {
                    inChar = true;
                    ch++;
                    continue;
               } else if(ret.at(ch) == QChar('\'') && inChar && !inQuote) {
                    inChar = false;
                    ch++;
                    continue;
               } else if(!inChar && inQuote && ret.at(ch) == QChar('\"')) {
                    inQuote = false;
                    qLen = ch - qStart + 1;
                    ret.remove(qStart, qLen);
                    qLen = 0;
                    qStart = 0;
                    ch = 0;
               } else if(ch+1 == ret.size() && inQuote) {
                    inQuote = false;
                    qLen = ret.size() - qStart - 1;
                    ret.remove(qStart, qLen);
                    qLen = 0;
                    qStart = 0;
                    ch = 0;
               }
               ch++;
          }
          ch = 0;
          qLen = 0;
          qStart = 0;
          inChar = false;
          while(ch < ret.size()) {
               if(ret.at(ch) == QChar('\'') && !inChar) {
                    qStart = ch++;
                    inChar = true;
                    continue;
               } else if(ret.at(ch) == QChar('\\') && inChar) {
                    ch+=2;
                    continue;
               } else if(inChar && ret.at(ch) == QChar('\'')) {
                    inChar = false;
                    qLen = ch - qStart + 1;
                    ret.remove(qStart, qLen);
                    qLen = 0;
                    qStart = 0;
                    ch = 0;
               }
               ch++;
          }
          return ret.simplified();
     }

     static int mCommentStart(const QString &text, int from = 0)
     {
          int i = from;
          bool inQuote = false, inChar = false;
          while(i < text.size()) {
               if(text.at(i) == '\"' && !inQuote && !inChar)
                    inQuote = true;
               else if(text.at(i) == '\"' && inQuote && !inChar)
                    inQuote = false;

               if(text.at(i) == '\'' && !inChar && !inQuote)
                    inChar = true;
               else if(text.at(i) == '\'' && inChar && !inQuote)
                    inChar = false;

               if(text.at(i) == '\\')
                    i++;
               
               if(text.at(i) == '/' && text.at(qBound(from, i+1, text.size()-1)) == '/' && qBound(from, i+1, text.size()-1) != i && !inQuote && !inChar) // //* is NOT a comment
                    return -1;

               if(text.at(i) == '/' && text.at(qBound(from, i+1, text.size()-1)) == '*' && qBound(from, i+1, text.size()-1) != i && !inQuote && !inChar)
                    return i;
               i++;
          }
          return -1;
     }

     static int mCommentEnd(const QString &text, int from = 0)
     {
          int i = from;
          bool inQuote = false, inChar = false;
          while(i < text.size()) {
               if(text.at(i) == '\"' && !inQuote && !inChar)
                    inQuote = true;
               else if(text.at(i) == '\"' && inQuote && !inChar)
                    inQuote = false;

               if(text.at(i) == '\'' && !inChar && !inQuote)
                    inChar = true;
               else if(text.at(i) == '\'' && inChar && !inQuote)
                    inChar = false;

               if(text.at(i) == '\\')
                    i++;

               if(!inQuote && !inChar && text.at(qBound(from,i,text.size()-1)) == '*' && text.at(qBound(from, i+1, text.size()-1)) == '/' && qBound(from, i+1, text.size()-1) != i)
                    return i;
               i++;
          }
          return -1;
     }

     static int lastStringIndex(const QString &text)
     {
          int i = 0, c = 0;
          while(i < text.size()) {
               if(text.at(i) == '\"' || text.at(i) == '\'')
                    c = i;
               if(text.at(i) == '\\')
                    i++;
               i++;
          }
          return c;
     }

     static int commentIndex(const QString &text)
     {
          int i;
          int c;
          bool inQuote;
          bool inChar;
          i = 0;
          c = -1;
          inQuote = false;
          inChar = false;
          while(i < text.size()) {
               if(text.at(i) == '\"' && !inQuote) {
                    i++;
                    inQuote = true;
                    continue;
               }
               if(text.at(i) == '\"' && inQuote) {
                    i++;
                    inQuote = false;
                    continue;
               }

               if(text.at(i) == '\'' && !inChar) {
                    i++;
                    inChar = true;
                    continue;
               }
               if(text.at(i) == '\'' && inChar) {
                    i++;
                    inChar = false;
                    continue;
               }

               if(text.at(i) == '/' && text.at(qBound(0,i+1,text.size()-1)) == '/' && !inChar && !inQuote ) {
                    c = i;
                    break;
               }
               if(text.at(i) == '\\')
                    i++;
               i++;
          }
          return (c == -1) ? text.size() : c;
     }

     static bool allParenthesisClosed(QString &text)
     {
          int i = 0, c = 0;
          bool inQuote = false, inChar = false;
          while(i < text.size()) {
               if(text.at(i) == '(' && !inQuote && !inChar)
                    c++; //lol
               if(text.at(i) == ')' && !inQuote && !inChar)
                    c--;

               if(text.at(i) == '\"' && !inQuote)
                    inQuote = true;
               else if(text.at(i) == '\"' && inQuote)
                    inQuote = false;

               if(text.at(i) == '\'' && !inChar)
                    inChar = true;
               else if(text.at(i) == '\'' && inChar)
                    inChar = false;

               if(text.at(i) == '\\')
                    i++;
               i++;
          }
          return (c == 0);
     }

public slots:
     void setActive(bool t) { off = !t; }

protected:
     bool eventFilter(QObject *target, QEvent *event);

private:
     int getIndentForLine(int lineNum);
     QTextEdit* edit;
     bool autoInsertion;
     int lastRow;
     int preLastRow;
     int depth;
     bool off;
};

#endif

