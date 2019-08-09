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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QShowEvent;
class QButtonGroup;
class QRadioButton;

class FindDialog: public QDialog
{
     Q_OBJECT
               
     public:
          FindDialog(QWidget *parent = 0);
          ~FindDialog();
          
     signals:
          void find(const QRegExp &exp, QTextDocument::FindFlags options);
          
     protected:
          void showEvent(QShowEvent *event);
          
     private slots:
          void findClicked();
          void enableFindButton(const QString &text);
          
     private:
          QWidget *par;
          int x, y;
          QLabel *label;
          QLineEdit *searchTerm;
          QCheckBox *caseCheckBox;
          QCheckBox *backCheckBox;
          QButtonGroup *styleGroup;
          QRadioButton *fixedOpt;
          QRadioButton *regExpOpt;
          QRadioButton *wildcardOpt;
          QPushButton *findButton;
          QPushButton *closeButton;
};

#endif
