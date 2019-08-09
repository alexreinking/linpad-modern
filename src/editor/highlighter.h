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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include "lpsd/lpsd.h"

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
     Q_OBJECT

public:
     Highlighter(QTextDocument *parent = 0);
     void setSourceType(const QString &text);
     QString type() { return typeOfSource; }

signals:
     void indentChanged(bool t);

protected:
     void highlightBlock(const QString &text);

private:
     int indexInAcceptableRanges(QString text, QRegExp exp, QList<QPair<int, int> > ranges, int offset = 0);
     LPSDReader reader;

     QString typeOfSource;
     int state;
};

#endif
