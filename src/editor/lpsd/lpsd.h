#ifndef LPSD_H
#define LPSD_H

//#define LPSD_DEBUG

#include <QObject>
#include <QStringList>
#include "lpsdformat.h"
#include "lpsdsequence.h"
#include "lpsdword.h"
class QString;
class QDomElement;

class LPSDReader : public QObject
{
     Q_OBJECT
public:
     LPSDReader(const QString &f = "") : file(f),indent(false) {}
     ~LPSDReader() {}
     bool parse(const QString &f = "");
     bool setFileType(QString ext);
     bool useCIndentation() { return indent; }
     const QString getFile() { return file; }
     const QString getFileType() { return fileType; }
     const QStringList getFileExtensions() { return types; }
     const QList<LPSDFormat> getFormats() { return formats; }
     const QList<LPSDSequence> getSequences() { return sequences; }
     const QList<LPSDWord> getWords() { return words; }
     
signals:
     void done(); //If multithreaded
     
private:
     //Convenience, pure convenience
     bool performConfiguration();
     bool performParse();
     //Syntax Style Parsers
     void parseRootElement(const QDomElement &element);
     void parseListElement(const QDomElement &element);
     void parseWordElement(const QDomElement &element);
     //Sequence parsers
     void parseSequenceElement(const QDomElement &element);
     void parseStartElement(const QDomElement &element, LPSDSequence &seq);
     void parseStopElement(const QDomElement &element, LPSDSequence &seq);
     //Configuration parsers
     void parseConfigElement(const QDomElement &element);
     void parseFormatElement(const QDomElement &element, LPSDFormat& format);
     void parseBoldElement(const QDomElement &element, LPSDFormat& format);
     void parseItalicElement(const QDomElement &element, LPSDFormat& format);
     void parseUnderlineElement(const QDomElement &element, LPSDFormat& format);
     void parseColorElement(const QDomElement &element, LPSDFormat& format);
     //File information
     QString file;
     QString fileType;
     QStringList types;
     QList<LPSDFormat> formats;
     QList<LPSDSequence> sequences;
     QList<LPSDWord> words;
     QStringList acceptableFormats;
     bool indent;
     //Add in private "LPSDToken" list.
};

#endif
