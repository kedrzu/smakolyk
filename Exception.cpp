#include "Exception.h"

Exception::Exception()
{
}

QString Exception::toHtml() const
{
    return headerToHtml().append(stackToHtml());
}

QString Exception::toString() const
{

    return headerToString().append(stackToString());
}

QString Exception::headerToHtml() const
{
    QString html = QString::fromUtf8("<table cellspacing='5'><tr><td colspan=2><b><u>Błąd</u></b></td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Typ błędu: </b></td><td>")+type+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Wiadomość: </b></td><td>")+msg+"</td></tr>");
    html.append("</table>\n");
    return html;
}

QString Exception::headerToString() const
{
    QString str = QString::fromUtf8("\tBłąd\n");
    str += QString::fromUtf8("Typ błędu:\t")+type+"\n";
    str += QString::fromUtf8("Wiadomość:\t")+msg+"\n";
    return str;
}

QString Exception::stackToHtml() const
{
    QString html;
    if(stack.size()>0) {
        html.append("<table cellspacing='5'><tr><td colspan=2><b><u>Stos</u></b></td></tr><tr><td><b>Plik</b></td><td><b>Metoda</b></td></tr>");
    for(int i=0; i<stack.size(); ++i)
        html.append("<tr><td>" + stack.at(i).file + ":" + QString::number(stack.at(i).line) + "</td><td>" + stack.at(i).name +"</td></tr>");
    html.append("</table>");
    }
    return html;
}

QString Exception::stackToString() const
{
    QString str;
    if(stack.size()>0)
        str.append("Stos:\n");
    for(int i=0; i<stack.size(); ++i)
        str.append(stack.at(i).file + ":" + QString::number(stack.at(i).line) + "\t" + stack.at(i).name +"\n");
    return str;
}

void Exception::pushStackTrace(QString name, QString file, uint line)
{
    stack << StackTraceItem(name, file, line);
}
