@if "%{GITHUBLINK}" == "true"
// %{GITHUBURL}/%{ProjectName}
@endif
@if "%{QT}" == "true"
@if "%{CONSOLE}" == "false"
@if "%{QT4SUPPORT}" == "false"
#include <QtWidgets>
@else
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
@endif
@else
#include <QtCore>
@endif

@if "%{OBJECT}" != ""
class %{OBJECT} : public QObject {
    Q_OBJECT
public:
@if "%{CPP11INITS}" == "true"
    %{OBJECT}(QObject * parent = 0) : QObject{parent} {}
@else
    %{OBJECT}(QObject * parent = 0) : QObject(parent) {}
@endif
};

@endif
@if "%{SOQUESTION}" != ""
// Code from SO question %{SOQUESTION}
%{JS: StackOverflow.getQuestionCode("%{SOQUESTION}")}
// Status
%{JS: StackOverflow.getStatus()}

@endif
int main(int argc, char ** argv) {
@if "%{CONSOLE}" == "false"
@if "%{CPP11INITS}" == "true"
    QApplication app{argc, argv};
@else
    QApplication app(argc, argv);
@endif
@else
@if "%{CPP11INITS}" == "true"
    QCoreApplication app{argc, argv};
@else
    QCoreApplication app(argc, argv);
@endif
@endif
    return app.exec();
}
@if "%{OBJECT}" != ""

#include "main.moc"
@endif
@else
#include <iostream>

@if "%{SOQUESTION}" != ""
// Code from SO question %{SOQUESTION}
%{JS: StackOverflow.getQuestionCode("%{SOQUESTION}")}
// Status
%{JS: StackOverflow.getStatus()}

@endif
int main()
{
}
@endif
