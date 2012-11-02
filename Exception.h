#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QList>
#include <QString>

#define StackTrace(EXCEPTION, NAME) \
    EXCEPTION.pushStackTrace(NAME,__FILE__,__LINE__);

#define StackTraceBegin() \
    try {

#define StackTraceEnd(NAME) \
    } \
    catch (Exception& e) { \
    e.pushStackTrace(NAME,__FILE__,__LINE__); \
    throw; \
    }

class Exception
{
public:
    struct StackTraceItem {
        QString name;
        QString file;
        uint line;
        StackTraceItem() {}
        StackTraceItem(QString name, QString file, uint line) :
            name(name), file(file), line(line) {}
    };

    QString msg;
    QList<StackTraceItem> stack;

    Exception();
    virtual QString toHtml() const;
    virtual QString toString() const;
    QString stackToHtml() const;
    QString stackToString() const;
    void pushStackTrace(QString name, QString file, uint line);
};

#endif // EXCEPTION_H
