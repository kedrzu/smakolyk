#include "Exception.h"

Exception::Exception()
{
}

QString Exception::toHtml() const
{
    QString html = "<table cellspacing='5'>";
    html.append(QString::fromUtf8("<tr><td><b>Wiadomość: </b></td><td>")+msg+"</td></tr>");
    html.append("</table>\n");
    html.append(stackToHtml());
    return html;
}

QString Exception::toString() const
{
    QString str = QString::fromUtf8("Wiadomość:\t")+msg+"\n";
    str.append(stackToString());
    return str;
}

QString Exception::stackToHtml() const
{
    QString html;
    if(stack.size()>0)
        html.append("<b><u>Stos:</u></b>\n");
    for(int i=0; i<stack.size(); ++i)
        html.append(stack.at(i).file + ":" + QString::number(stack.at(i).line) + "\t" + stack.at(i).name +"\n");
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
