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

#include <QApplication>
#include <iostream>
#ifdef Q_OS_LINUX
#include <unistd.h>
#include <signal.h>
#endif
#include "linpad.h"
using namespace std;

#ifdef Q_OS_LINUX
void catch_int(int sig_num)
{
    Q_UNUSED(sig_num)
     cout << "SIGINT Caught!" << endl << "Exiting...." << endl << flush;
     exit(0);
}
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
     signal(SIGINT, catch_int); //SIGINT Handler
#endif
     QApplication app(argc, argv);
     QStringList args = app.arguments();
     
     LinPad mainWin;
     if (args.count() > 1) {
          args[0] = "";
          mainWin.openFile(args);
     } else {
          mainWin.newFile();
     }

     mainWin.show();
     return app.exec();
}
