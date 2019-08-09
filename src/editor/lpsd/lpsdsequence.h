//Format definition container.
#ifndef LPSD_SEQUENCE_H
#define LPSD_SEQUENCE_H

#include <QRegExp>
#include <QString>
#include "lpsdformat.h"

class LPSDSequence
{
public:
     LPSDSequence(int p = 99, QString i = "", QRegExp exp = QRegExp(""), QRegExp expE = QRegExp(""), LPSDFormat f = LPSDFormat(), bool m = false, int t = 1):priority(p),trail(t),id(i),format(f),multi(m),start(exp),stop(expE) {}
     
     LPSDFormat getFormat() { return format; }
     QRegExp getStart() { return start; }
     QRegExp getStop() { return stop; }
     const QString getId() { return id;}
     const QString getEscapes() { return escapes;}
     int getPriority() { return priority;}
     bool getMultiline() { return multi; }
     int getTrail() { return trail; }
     
     void setFormat(const LPSDFormat f) { format = f; }
     void setStart(const QString r) { start = QRegExp(r); }
     void setStop(const QString o) { stop = QRegExp(o); }
     void setId(const QString s) { id = s;}
     void setEscapes(const QString e) { escapes = e;}
     void setPriority(int p) { priority = p; }
     void setMultiline(bool m) { multi = m; }
     void setTrail(int t) { trail = t; }
     
     bool operator==(LPSDSequence ref)
     {
          if(ref.getFormat() == format) {
               if(ref.getStart() == start) {
                    if(ref.getStop() == stop) {
                         if(ref.getId() == id) {
                              if(ref.getEscapes() == escapes) {
                                   if(ref.getPriority() == priority) {
                                        return true;
                                   }
                              }
                         }
                    }
               }
          }
          return false;
     }
     
private:
     int priority, trail;
     QString id;
     QString escapes;
     LPSDFormat format;
     bool multi;
     QRegExp start, stop;
};


#endif
