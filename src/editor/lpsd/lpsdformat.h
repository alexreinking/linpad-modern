//Format definition container.
#ifndef LPSD_FORMAT_H
#define LPSD_FORMAT_H

#include <QColor>
#include <QString>
#include <QTextCharFormat>

class LPSDFormat
{
public:
     LPSDFormat(QString id = "", QColor clr = QColor(0,0,0), bool b = false, bool i = false, bool u = false):id(id),color(clr),bold(b),italic(i),underline(u) {}
     
     QTextCharFormat asTextCharFormat()
     {
          QTextCharFormat ret;
          ret.setForeground(color);
          if(bold) ret.setFontWeight(99);
          ret.setFontItalic(italic);
          ret.setFontUnderline(underline);
          return ret;
     }
     
     const QColor getColor() { return color; }
     bool getBold() { return bold; }
     bool getItalic() { return italic; }
     bool getUnderline() { return underline; }
     const QString getId() { return id;}
     
     void setColor(const QColor c) { color = c; }
     void setBold(const bool b) { bold = b; }
     void setItalic(const bool i) { italic = i; }
     void setUnderline(const bool u) { underline = u; }
     void setId(const QString s) { id = s;}
     
     bool operator==(LPSDFormat ref)
     {
          if(ref.getId() == id) {
               if(ref.getBold() == bold) {
                    if(ref.getItalic() == italic) {
                         if(ref.getUnderline() == underline) {
                              if(ref.getColor() == color) {
                                   return true;
                              }
                         }
                    }
               }
          }
          return false;
     }
     
private:
     QString id;
     QColor color;
     bool bold, italic, underline;
};

#endif
