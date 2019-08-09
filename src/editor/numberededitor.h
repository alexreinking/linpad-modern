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

#ifndef NUMBERED_EDITOR_H
#define NUMBERED_EDITOR_H

#include <QFrame>
#include <QPixmap>
#include <QTextCursor>
#include <QList>
#include <QVector>
class Editor;
class QHBoxLayout;
class QCloseEvent;
class QMouseEvent;

class NumberBar : public QWidget
{
     Q_OBJECT
     public:
          NumberBar( QWidget *parent );

          void setEditor( Editor *edit );
          void paintEvent( QPaintEvent *ev );
          QList<int> getBookmarks() { return bookmarks; }
          void bookmark(int mark);
          void removeAllBookmarks();

     signals:
          void bookmarksChanged();
     
     protected:
          void mouseReleaseEvent(QMouseEvent *event);
     
     private:
          Editor *edit;
          bool showing;
          QList<int> bookmarks; //Removable
          QVector<int> lineHeights;
};

class NumberedEditor : public QFrame
{
     Q_OBJECT
     public:
          NumberedEditor( QWidget *parent );
          ~NumberedEditor();
          Editor *editor() const { return view; }
          NumberBar *numberBar() const { return numbers; } //Removable
          void setShowing( bool show );

     protected:
          void closeEvent(QCloseEvent*);
          
     private:
          Editor *view;
          NumberBar *numbers;
          QHBoxLayout *box;
};

#endif
