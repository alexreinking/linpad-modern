//Format definition container.
#ifndef LPSD_WORD_H
#define LPSD_WORD_H

#include <QRegExp>
#include <QString>
#include "lpsdformat.h"

class LPSDWord
{
public:
     LPSDWord(QString l = "", QRegExp exp = QRegExp(""), LPSDFormat f = LPSDFormat()):format(f),expr(exp),list(l) {}
     
     LPSDFormat getFormat() { return format; }
     QRegExp getExpression() { return expr; }
     const QString getList() { return list; }
     
     void setFormat(const LPSDFormat f) { format = f; }
     void setExpression(const QRegExp r) { expr = r; }
     void setList(const QString s) { list = s; }
     
     bool operator==(LPSDWord ref)
     {
          if(ref.getFormat() == format) {
               if(ref.getExpression() == expr) {
                    if(ref.getList() == list) {
                         return true;
                    }
               }
          }
          return false;
     }
     
private:
     LPSDFormat format;
     QRegExp expr;
     QString list;
};

#endif
