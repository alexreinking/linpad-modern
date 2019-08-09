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
#include "findDialog.h"

FindDialog::FindDialog(QWidget *parent) : QDialog(parent)
{
     par = parent;
     label = new QLabel(tr("Find &what:"));
     searchTerm = new QLineEdit;
     label->setBuddy(searchTerm);
     
     caseCheckBox = new QCheckBox(tr("Match &case?"));
     backCheckBox = new QCheckBox(tr("Search &backward?"));
     
     findButton = new QPushButton(tr("&Find"));
     findButton->setDefault(true);
     findButton->setEnabled(false);
     
     closeButton = new QPushButton(tr("Close"));
     
     styleGroup = new QButtonGroup;
     
     fixedOpt = new QRadioButton(tr("Fixed &String"));
     regExpOpt = new QRadioButton(tr("Regular &Expression"));
     wildcardOpt = new QRadioButton(tr("&Wildcard"));
     fixedOpt->setChecked(true);
     
     styleGroup->addButton(fixedOpt, 0);
     styleGroup->addButton(regExpOpt, 1);
     styleGroup->addButton(wildcardOpt, 2);
     
     connect(searchTerm, SIGNAL(textChanged(const QString&)), this, SLOT(enableFindButton(const QString&)));
     connect(findButton, SIGNAL(clicked()), this, SLOT(findClicked()));
     connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
     
     QHBoxLayout *topLayout = new QHBoxLayout;
     topLayout->addWidget(label);
     topLayout->addWidget(searchTerm);
     
     QHBoxLayout *expTypeLayout = new QHBoxLayout;
     expTypeLayout->addWidget(fixedOpt);
     expTypeLayout->addWidget(regExpOpt);
     expTypeLayout->addWidget(wildcardOpt);
     expTypeLayout->addStretch();
     
     QHBoxLayout *firstLayout = new QHBoxLayout;
     firstLayout->addWidget(caseCheckBox);
     firstLayout->addWidget(backCheckBox);
     firstLayout->addStretch();
     
     QHBoxLayout *secondLayout = new QHBoxLayout;
     secondLayout->addWidget(findButton);
     secondLayout->addWidget(closeButton);
     secondLayout->addStretch();
     
     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->addLayout(topLayout);
     mainLayout->addLayout(expTypeLayout);
     mainLayout->addLayout(firstLayout);
     mainLayout->addLayout(secondLayout);
     setLayout(mainLayout);
     
     //QRect geometry = QApplication::desktop()->screenGeometry();
     x = par->x() + par->width() - this->width()-10;
     y = par->y() + par->height() - this->height()-10;
     
     move(x,y);
     
     setWindowTitle(tr("Find"));
     setFixedHeight(sizeHint().height());
}

FindDialog::~FindDialog()
{
     delete label;
     delete searchTerm;
     delete caseCheckBox;
     delete backCheckBox;
     delete styleGroup;
     delete fixedOpt;
     delete regExpOpt;
     delete wildcardOpt;
     delete findButton;
     delete closeButton;
}

void FindDialog::enableFindButton(const QString &text)
{
     findButton->setEnabled(!text.isEmpty());
}

void FindDialog::findClicked()
{
     QRegExp text(searchTerm->text());
     text.setCaseSensitivity(Qt::CaseInsensitive);
     QTextDocument::FindFlags options = 0;
     
     if(styleGroup->checkedId() == 0)
          text.setPatternSyntax(QRegExp::FixedString);
     if(styleGroup->checkedId() == 1)
          text.setPatternSyntax(QRegExp::RegExp);
     if(styleGroup->checkedId() == 2)
          text.setPatternSyntax(QRegExp::Wildcard);
     
     if( caseCheckBox->isChecked() )
     {
          text.setCaseSensitivity(Qt::CaseSensitive);
     }
     if( backCheckBox->isChecked() )
     {
          options = QTextDocument::FindBackward;
     }
     
     emit find(text,options);
}

void FindDialog::showEvent(QShowEvent *event)
{
     x = par->x() + par->width() - this->width()-10;
     y = par->y() + par->height() - this->height()-10;
     move(x,y);
     event->accept();
}
