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
#include "replaceDialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) : QDialog(parent)
{
     par = parent;
     label = new QLabel(tr("Find &what:"));
     searchTerm = new QLineEdit;
     label->setBuddy(searchTerm);
     
     replaceLabel = new QLabel(tr("R&eplace with:"));
     replaceTerm = new QLineEdit;
     replaceTerm->setReadOnly(true);
     replaceLabel->setBuddy(replaceTerm);
     
     caseCheckBox = new QCheckBox(tr("Match &case?"));
     backCheckBox = new QCheckBox(tr("Search &backward?"));
     replaceAllCheckBox = new QCheckBox(tr("Replace &All?"));
     
     findButton = new QPushButton(tr("&Replace"));
     findButton->setDefault(true);
     findButton->setEnabled(false);
     
     closeButton = new QPushButton(tr("Close"));
     
     styleGroup = new QButtonGroup;
     
     fixedOpt = new QRadioButton(tr("Fixed &String"));
     regExpOpt = new QRadioButton(tr("Regular &Expression"));
     wildcardOpt = new QRadioButton(tr("&Wildcard"));
     
     styleGroup->addButton(fixedOpt, 0);
     styleGroup->addButton(regExpOpt, 1);
     styleGroup->addButton(wildcardOpt, 2);
     
     fixedOpt->setChecked(true);
     
     connect(searchTerm, SIGNAL(textChanged(const QString&)), this, SLOT(enableReplaceField(const QString&)));
     connect(replaceTerm, SIGNAL(textChanged(const QString&)), this, SLOT(enableReplaceButton(const QString&)));
     connect(findButton, SIGNAL(clicked()), this, SLOT(replaceClicked()));
     connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
     
     QHBoxLayout *topLayout = new QHBoxLayout;
     topLayout->addWidget(label);
     topLayout->addWidget(searchTerm);
     
     QHBoxLayout *expTypeLayout = new QHBoxLayout;
     expTypeLayout->addWidget(fixedOpt);
     expTypeLayout->addWidget(regExpOpt);
     expTypeLayout->addWidget(wildcardOpt);
     
     QHBoxLayout *firstLayout = new QHBoxLayout;
     firstLayout->addWidget(replaceLabel);
     firstLayout->addWidget(replaceTerm);
     
     QHBoxLayout *secondLayout = new QHBoxLayout;
     secondLayout->addWidget(caseCheckBox);
     secondLayout->addWidget(backCheckBox);
     secondLayout->addWidget(replaceAllCheckBox);
     
     QHBoxLayout *thirdLayout = new QHBoxLayout;
     thirdLayout->addStretch();
     thirdLayout->addWidget(findButton);
     thirdLayout->addWidget(closeButton);
     
     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->addLayout(topLayout);
     mainLayout->addLayout(expTypeLayout);
     mainLayout->addLayout(firstLayout);
     mainLayout->addLayout(secondLayout);
     mainLayout->addLayout(thirdLayout);
     setLayout(mainLayout);
     
     //QRect geometry = QApplication::desktop()->screenGeometry();
     x = par->x() + par->width() - this->width()-10;
     y = par->y() + par->height() - this->height()-10;
     
     move(x,y);
     
     setWindowTitle(tr("Find and Replace"));
     setFixedHeight(sizeHint().height());
}

ReplaceDialog::~ReplaceDialog()
{
     delete label;
     delete replaceLabel;
     delete searchTerm;
     delete replaceTerm;
     delete caseCheckBox;
     delete backCheckBox;
     delete replaceAllCheckBox;
     delete styleGroup;
     delete fixedOpt;
     delete regExpOpt;
     delete wildcardOpt;
     delete findButton;
     delete closeButton;
}

void ReplaceDialog::enableReplaceButton(const QString &text)
{
     findButton->setEnabled(!text.isEmpty());
}

void ReplaceDialog::enableReplaceField(const QString &text)
{
     replaceTerm->setReadOnly(text.isEmpty());
     if(text.isEmpty())
          replaceTerm->setText("");
}

void ReplaceDialog::replaceClicked()
{
     QRegExp text(searchTerm->text());
     text.setCaseSensitivity(Qt::CaseInsensitive);
     QString rep = replaceTerm->text();
     unsigned int options = 0;
     
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
          options |= 0x00001;
     }
     if(replaceAllCheckBox->isChecked())
     {
          options |= 0x00002;
     }
     
     emit replace(text,rep,options);
}

void ReplaceDialog::showEvent(QShowEvent *event)
{
     x = par->x() + par->width() - this->width()-10;
     y = par->y() + par->height() - this->height()-10;
     move(x,y);
     event->accept();
}
