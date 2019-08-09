 /**************************************************************************
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

#include <QtGui>
#include <iostream>
#include "highlighter.h"
typedef QPair<int, int> intpair;

Highlighter::Highlighter(QTextDocument *parent)
          : QSyntaxHighlighter(parent)
{ typeOfSource = ""; state = -1;}


bool lpsdLessThan(LPSDSequence &a, LPSDSequence &b)
{
     if(a.getPriority() < b.getPriority())
          return true;
     return false;
}

void Highlighter::highlightBlock(const QString &text)
{
     if(typeOfSource != "") {
          foreach (LPSDWord word, reader.getWords()) {
               QRegExp expression(word.getExpression());
               int index = text.indexOf(expression);
               while (index >= 0) {
                    int length = expression.matchedLength();
                    setFormat(index, length, word.getFormat().asTextCharFormat());
                    index = text.indexOf(expression, index + length);
               }
          }
          
          QList<LPSDSequence> seqs = reader.getSequences();
          QList<QPair<int, int> > ranges;
          qSort(seqs.begin(), seqs.end(), lpsdLessThan);
          int index = 0;
          int start = 0;
          while(index < text.size()) //single line only
          {
               begin:
               if(state == -1) { //No block opened
                    foreach(LPSDSequence seq, seqs) {
                         if(seq.getMultiline()) continue;
                         QRegExp expression(seq.getStart());
                         int i = text.indexOf(expression, index);
                         if(index == i) {
                              state = seq.getPriority();
                              start = i;
                              goto begin; //Breaks cleanly from a loop, sue me.
                         }
                    }
               } else { //Block opened
                    foreach(LPSDSequence seq, seqs) {
                         if(seq.getMultiline()) continue;
                         QRegExp expression(seq.getStop());
                         if(state == seq.getPriority()) //Same block as state
                         {
                              int end = indexInAcceptableRanges(text, expression, ranges, index+1);
                              if(index == 0) end = indexInAcceptableRanges(text, expression, ranges, seq.getTrail());
                              if(end != -1 && end > index )
                              {
                                   if(!seq.getEscapes().contains(text.at(end-1)) || (seq.getEscapes().contains(text.at(end-2)) && seq.getEscapes().contains(text.at(end-1))) ) { //unescaped ending found
                                        index = end;
                                        state = -1;
                                        ranges << QPair<int, int>(start, index+seq.getTrail()-1);
                                        setFormat(start, index-start+seq.getTrail(), seq.getFormat().asTextCharFormat());
                                        start = 0;
                                   }
                              } else if(!seq.getMultiline()) { //no ending; not multiline
                                   state = -1;
                                   setFormat(start, text.length()-start, seq.getFormat().asTextCharFormat());
                              }
                         }
                    }
               }
               index++;
          }
          
          setCurrentBlockState(0);
          //Multi Line
          foreach(LPSDSequence seq, seqs)
          {
               if(!seq.getMultiline()) continue;
               QRegExp expression(seq.getStart());
               QRegExp endexpression(seq.getStop());
               int startIndex = 0;
               int endIndex = -1;

               if (previousBlockState() <= 0) //block state, if in a block will ALWAYS be greater than 0
                    startIndex = indexInAcceptableRanges(text, expression, ranges);
               else if(previousBlockState() != seq.getPriority())
                    continue;
               endIndex = indexInAcceptableRanges(text, endexpression, ranges, 0);
               while (startIndex >= 0) {
                    int offset = startIndex+seq.getTrail();
                    if(endIndex == -1) endIndex = indexInAcceptableRanges(text, endexpression, ranges, offset);
                    int commentLength;
                    if (endIndex == -1) {
                         setCurrentBlockState(seq.getPriority());
                         commentLength = text.length() - startIndex;
                    } else {
                         commentLength = endIndex - startIndex + seq.getTrail();
                         offset = endIndex+seq.getTrail()+1;
                    }
                    setFormat(startIndex, commentLength, seq.getFormat().asTextCharFormat());
                    startIndex = indexInAcceptableRanges(text, expression, ranges, offset);
               }
          }
     }
}

int Highlighter::indexInAcceptableRanges(QString text, QRegExp exp, QList<QPair<int, int> > ranges, int offset)
{
     int ret = text.indexOf(exp, offset);
     if(ret == -1)
          return -1;
     foreach(intpair /* QList<QPair<int, int> > */ range, ranges)
     {
          if(ret >= range.first && ret <= range.second)
          {
               return indexInAcceptableRanges(text, exp, ranges, range.second + 1);
          }
     }
     return ret;
}

void Highlighter::setSourceType(const QString &text)
{
     bool t = reader.setFileType(text);
     if(t) typeOfSource = text;
     emit indentChanged(reader.useCIndentation());
     rehighlight();
}
