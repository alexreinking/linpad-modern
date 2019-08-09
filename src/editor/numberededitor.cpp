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

#include "numberededitor.h"
#include "editor.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextEdit>
#include <QBoxLayout>
#include <QScrollBar>
#include <QPainter>
#include <QTextObjectInterface>
#include <QToolTip>
#include <QMouseEvent>
#include <cmath>
#include <iostream>
using std::cout;
using std::endl;

NumberBar::NumberBar( QWidget *parent ) : QWidget( parent ), edit(0)
{
     setFixedWidth( fontMetrics().width( QString("0000") + 10 + 32 ) );
}

void NumberBar::bookmark(int mark)
{
     if(bookmarks.contains(mark))
          bookmarks.removeAll(mark);
     else
          bookmarks.append(mark);
     qSort(bookmarks);
     repaint();
     emit bookmarksChanged();
}

void NumberBar::removeAllBookmarks()
{
     bookmarks.clear();
     repaint();
     emit bookmarksChanged();
}

void NumberBar::setEditor( Editor *edit )
{
     this->edit = edit;
     connect( edit->document()->documentLayout(), SIGNAL( update(const QRectF &) ),
              this, SLOT( update() ) );
     connect( edit->verticalScrollBar(), SIGNAL(valueChanged(int) ),
              this, SLOT( update() ) );
}

void NumberBar::paintEvent( QPaintEvent * )
{
     QAbstractTextDocumentLayout *layout = edit->document()->documentLayout();
     int contentsY = edit->verticalScrollBar()->value();
     qreal pageBottom = contentsY + edit->viewport()->height();
     const QFontMetrics fm = fontMetrics();
     const int ascent = fontMetrics().ascent() + 1; // height = ascent + descent + 1
     int lineCount = 1;
     
     QPainter p(this);
     for ( QTextBlock block = edit->document()->begin();
             block.isValid(); block = block.next(), ++lineCount ) {

          const QRectF boundingRect = layout->blockBoundingRect( block );

          QPointF position = boundingRect.topLeft();
          if ( position.y() + boundingRect.height() < contentsY )
               continue;
          if ( position.y() > pageBottom )
               break;

          const QString txt = QString::number( lineCount );
          if(bookmarks.contains(lineCount))
          {
               QColor highlight = p.brush().color();
               highlight.setRgb(51,102,255,50);
               p.setBrush(QBrush(highlight));
               p.drawRect(0,qRound(position.y()) - contentsY, width(), fm.height()); //x,y,width,height
          }
          p.drawText( width() - fm.width(txt), qRound( position.y() ) - contentsY + ascent, txt );
     }
}

void NumberBar::mouseReleaseEvent(QMouseEvent *event)
{
     QAbstractTextDocumentLayout *layout = edit->document()->documentLayout();
     int contentsY = edit->verticalScrollBar()->value();
     const QFontMetrics fm = fontMetrics();
     const int ascent = fontMetrics().ascent() + 1; // height = ascent + descent + 1
     int lineCount = 1;
     
     for ( QTextBlock block = edit->document()->begin(); block.isValid(); block = block.next(), ++lineCount ) {

          const QRectF boundingRect = layout->blockBoundingRect( block );

          QPointF position = boundingRect.topLeft();
          if( qRound( position.y() ) - contentsY + ascent > event->y() ) {
               bookmark(lineCount);
               break;
          }
     }
}

NumberedEditor::NumberedEditor( QWidget *parent = 0 ) : QFrame( parent )
{
     setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
     setLineWidth( 2 );

     // Setup the main view
     view = new Editor(0,this);
     view->setAcceptDrops(false);

     // Setup the line number pane
     numbers = new NumberBar( this );
     numbers->setEditor( view );

     //Layout time
     box = new QHBoxLayout( this );
     box->setSpacing( 0 );
     box->setMargin( 0 );
     box->addWidget( numbers );
     box->addWidget( view );
     resize(750,500);
     setAttribute(Qt::WA_DeleteOnClose);
}

NumberedEditor::~NumberedEditor()
{
     delete numbers;
     delete view;
     delete box;
}

void NumberedEditor::setShowing( bool show )
{
     if(!show)
          numbers->hide();
     else
          numbers->show();
}

void NumberedEditor::closeEvent( QCloseEvent* event )
{
     bool closeable = view->close();
     if(closeable)
          event->accept();
     else
          event->ignore();
}
