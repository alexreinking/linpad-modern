#include "lpsd.h"
#include "lpsdformat.h"
#include <iostream>
#include <QtXml>

#define CONFIG_FILE "./config/config.lpsc"

bool LPSDReader::setFileType(QString ext)
{
     file = "";
     fileType = "";
     QDir dir("./formats");
     dir.setNameFilters((QStringList() << "*.lpsd"));
     QStringList files = dir.entryList();
     foreach(QString file, files)
     {
          parse("./formats/" + file);
          if(getFileExtensions().contains(ext))
               return true;
     }
     std::cerr << "No Proper Format detected" << std::endl;
     return false;
}

bool LPSDReader::parse(const QString &f)
{
     types.clear();
     formats.clear();
     sequences.clear();
     words.clear();
     acceptableFormats.clear();
     if(f != "") file = f;
     return (performConfiguration() && performParse());
}


bool LPSDReader::performConfiguration()
{
     QFile config(CONFIG_FILE);
     if(!config.open(QFile::ReadOnly | QFile::Text))
     {
          std::cerr << "Error: Cannot read file " << qPrintable(config.fileName())
                    << ": " << qPrintable(config.errorString()) << std::endl;
          return false;
     }
     
     QString cErrorStr;
     int cErrorLine, cErrorColumn;
     
     QDomDocument cDoc;
     if(!cDoc.setContent(&config, false, &cErrorStr, &cErrorLine, &cErrorColumn)) {
          std::cerr << "Error: Parse error at line " << cErrorLine << ", "
                    << "column " << cErrorColumn << ": " << qPrintable(cErrorStr) << std::endl;
          return false;
     }
     
     QDomElement cRoot = cDoc.documentElement();
     if(cRoot.tagName() != "lpsc") {
          std::cerr << "Error: Not a LPSC file" << std::endl;
          return false;
     }
     parseConfigElement(cRoot);
     return true;
}

bool LPSDReader::performParse()
{
     QFile source( file );
     
     if(!source.open(QFile::ReadOnly | QFile::Text))
     {
          std::cerr << "Error: Cannot read file " << qPrintable(source.fileName())
                    << ": " << qPrintable(source.errorString()) << std::endl;
          return false;
     }
     QString errorStr;
     int errorLine, errorColumn;
     
     QDomDocument doc;
     if(!doc.setContent(&source, false, &errorStr, &errorLine, &errorColumn)) {
          std::cerr << "Error: Parse error at line " << errorLine << ", "
                    << "column " << errorColumn << ": " << qPrintable(errorStr) << std:: endl;
          return false;
     }
     
     QDomElement root = doc.documentElement();
     if(root.tagName() != "lpsd") {
          std::cerr << "Error: Not a LPSD file" << std::endl;
          return false;
     }
     parseRootElement(root);
     return true;
}

void LPSDReader::parseRootElement(const QDomElement &element)
{
     fileType = element.attribute("type", "");
     types = element.attribute("extensions", "").split(";");
     indent = QVariant(element.attribute("cIndentation", "false")).toBool();
     
     QMutableStringListIterator typeSimplifier(types);
     while (typeSimplifier.hasNext()) {
          typeSimplifier.setValue(typeSimplifier.next().simplified());
     }
#ifdef LPSD_DEBUG
     std::cout << "-------------------" << std::endl;
     std::cout << "Root Element found!" << std:: endl;
     std::cout << "Reported file type: " << qPrintable(fileType) << std::endl;
     std::cout << "Supported file extentions:" << std::endl;
     QStringListIterator printer(types);
     while (printer.hasNext()) {
          std::cout << printer.next().toLocal8Bit().constData() << std::endl;
     }
     std::cout << std::endl;
#endif
     QDomNode child = element.firstChild();
     while(!child.isNull())
     {
          if(child.toElement().tagName() == "list")
               parseListElement(child.toElement());
          else if(child.toElement().tagName() == "word")
               parseWordElement(child.toElement());
          else if(child.toElement().tagName() == "sequence")
               parseSequenceElement(child.toElement());
          child = child.nextSibling();
     }
}

void LPSDReader::parseListElement(const QDomElement &element)
{
#ifdef LPSD_DEBUG
     std::cout << "List Element found!" << std::endl;
     std::cout << "Description: " << qPrintable(element.attribute("description", "")) << std::endl;
     std::cout << "Format: " << qPrintable(element.attribute("format", "")) << std::endl << std::endl;
#endif
     QDomNode child = element.firstChild();
     while(!child.isNull())
     {
          if(child.toElement().tagName() == "word")
               parseWordElement(child.toElement());
          child = child.nextSibling();
     }
}

void LPSDReader::parseWordElement(const QDomElement &element)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless word element!" << std::endl;
          return;
     }
     parent = par.toElement();
     if(parent.tagName() == "list")
     {
          if(!acceptableFormats.contains(parent.attribute("format", "")))
          {
               std::cout << "Error! Undefined format!" << std::endl;
               return;
          }
#ifdef LPSD_DEBUG
          std::cout << "Word Element found!" << std::endl;
          std::cout << "Description(Child of): " << qPrintable(parent.attribute("description", "")) << std::endl;
          std::cout << "Format: " << qPrintable(parent.attribute("format", "")) << std::endl;
#endif
          LPSDWord word;
          foreach(LPSDFormat l, formats)
          {
               if(l.getId() == parent.attribute("format", ""))
                    word.setFormat(l);
          }
          QRegExp exp = QRegExp(element.text());
          if(QVariant(element.attribute("caseSensitive", "true")).toBool())
               exp.setCaseSensitivity(Qt::CaseSensitive);
          else
               exp.setCaseSensitivity(Qt::CaseInsensitive);
          word.setExpression(exp);
          word.setList(parent.attribute("description", ""));
          words << word;
     }
     else
     {
          if(!acceptableFormats.contains(element.attribute("format", "")))
          {
               std::cout << "Error! Undefined format!" << std::endl;
               return;
          }
#ifdef LPSD_DEBUG
          std::cout << "Word Element found!" << std::endl;
          std::cout << "Description: " << qPrintable(element.attribute("description", "")) << std::endl;
          std::cout << "Format: " << qPrintable(element.attribute("format", "")) << std::endl;
#endif
          LPSDWord word;
          foreach(LPSDFormat l, formats)
          {
               if(l.getId() == element.attribute("format", ""))
                    word.setFormat(l);
          }
          QRegExp exp = QRegExp(element.text());
          if(QVariant(element.attribute("caseSensitive", "true")).toBool())
               exp.setCaseSensitivity(Qt::CaseSensitive);
          else
               exp.setCaseSensitivity(Qt::CaseInsensitive);
          word.setExpression(exp);
          word.setList(element.attribute("description", ""));
          words << word;
     }
#ifdef LPSD_DEBUG
     std::cout << "Value: " << qPrintable(element.text()) << std::endl << std::endl;
#endif
}

//Sequence routines

void LPSDReader::parseSequenceElement(const QDomElement &element)
{
     if(!acceptableFormats.contains(element.attribute("format", "")))
     {
          std::cout << "Error! Undefined format!" << std::endl;
          return;
     }
     LPSDSequence seq;
     foreach(LPSDFormat l, formats)
     {
          if(l.getId() == element.attribute("format", ""))
               seq.setFormat(l);
     }
     seq.setEscapes(element.attribute("escapes", ""));
     seq.setId(element.attribute("description", ""));
     seq.setPriority(QVariant(element.attribute("priority", "")).toInt());
     seq.setTrail(QVariant(element.attribute("trail", "1")).toInt());
     seq.setMultiline(QVariant(element.attribute("multiline", "false")).toBool());
     bool start = false, stop = false;
     QDomNode child = element.firstChild();
     while(!child.isNull())
     {
          if(child.toElement().tagName() == "start") {
               parseStartElement(child.toElement(), seq);
               start = true;
          }  else if(child.toElement().tagName() == "stop") {
               parseStopElement(child.toElement(), seq);
               stop = true;
          }
          child = child.nextSibling();
     }
     if (!(start && stop)) return;
     sequences << seq;
}

void LPSDReader::parseStartElement(const QDomElement &element, LPSDSequence &seq)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless start element!" << std::endl;
          return;
     }
     parent = par.toElement();
     seq.setStart(element.text());
}

void LPSDReader::parseStopElement(const QDomElement &element, LPSDSequence &seq)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless stop element!" << std::endl;
          return;
     }
     parent = par.toElement();
     seq.setStop(((element.text() == "\\n") ? "\n" : element.text()));
}

//Configuration routines

void LPSDReader::parseConfigElement(const QDomElement &element)
{
#ifdef LPSD_DEBUG
     std::cout << "Reading configuration file..." << std:: endl;
#endif
     QDomNode child = element.firstChild();
     while(!child.isNull())
     {
          LPSDFormat format;
          if(child.toElement().tagName() == "format")
               parseFormatElement(child.toElement(), format);
          if(!formats.contains(format))
               formats.append(format);
          child = child.nextSibling();
     }
#ifdef LPSD_DEBUG
     std::cout << "\nRunning through formats one last time..." << std::endl;
     QListIterator<LPSDFormat> f(formats);
     while (f.hasNext()) {
          LPSDFormat tmp = f.next();
          std::cout << qPrintable(tmp.getId()) << std::endl;
     }
     std::cout << std::endl;
#endif
}

void LPSDReader::parseFormatElement(const QDomElement &element, LPSDFormat &format)
{
#ifdef LPSD_DEBUG
     std::cout << "Format Element found!" << std::endl;
     std::cout << "Name: " << qPrintable(element.attribute("id", "")) << std::endl << std::endl;
#endif
     if(element.attribute("id", "") == "")
     {
          std::cerr << "Error!! Nameless Format Rule!" << std::endl;
          return;
     }
     acceptableFormats << element.attribute("id", "");
     format.setId(element.attribute("id", ""));
     QDomNode child = element.firstChild();
     while(!child.isNull())
     {
          if(child.toElement().tagName() == "bold")
               parseBoldElement(child.toElement(), format);
          if(child.toElement().tagName() == "italic")
               parseItalicElement(child.toElement(), format);
          if(child.toElement().tagName() == "underline")
               parseUnderlineElement(child.toElement(), format);
          if(child.toElement().tagName() == "color")
               parseColorElement(child.toElement(), format);
          child = child.nextSibling();
     }
}

void LPSDReader::parseBoldElement(const QDomElement &element, LPSDFormat &format)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless bold element!" << std::endl;
          return;
     }
     parent = par.toElement();
#ifdef LPSD_DEBUG
     std::cout << "Bold Element found!" << std::endl;
     std::cout << "Bolding Format: " << qPrintable(parent.attribute("id", "")) << std::endl << std::endl;
#endif
     QVariant val(element.text());
     format.setBold(val.toBool());
}

void LPSDReader::parseItalicElement(const QDomElement &element, LPSDFormat &format)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless italic element!" << std::endl;
          return;
     }
     parent = par.toElement();
#ifdef LPSD_DEBUG
     std::cout << "Italic Element found!" << std::endl;
     std::cout << "Italicising Format: " << qPrintable(parent.attribute("id", "")) << std::endl << std::endl;
#endif
     QVariant val(element.text());
     format.setItalic(val.toBool());
}

void LPSDReader::parseUnderlineElement(const QDomElement &element, LPSDFormat &format)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless underline element!" << std::endl;
          return;
     }
     parent = par.toElement();
#ifdef LPSD_DEBUG
     std::cout << "Underline Element found!" << std::endl;
     std::cout << "Underlineing Format: " << qPrintable(parent.attribute("id", "")) << std::endl << std::endl;
#endif
     QVariant val(element.text());
     format.setUnderline(val.toBool());
}

void LPSDReader::parseColorElement(const QDomElement &element, LPSDFormat &format)
{
     QDomNode par = element.parentNode();
     QDomElement parent;
     if(par.isNull())
     {
          std::cerr << "Error: Parentless color element!" << std::endl;
          return;
     }
     parent = par.toElement();
#ifdef LPSD_DEBUG
     std::cout << "Color Element found!" << std::endl;
     std::cout << "Coloring Format: " << qPrintable(parent.attribute("id", "")) << std::endl << std::endl;
#endif
     QColor val(element.text());
     format.setColor(val);
}
