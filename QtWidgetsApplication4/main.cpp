#include "stdafx.h"
#include "QTmodelviewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTModelViewer* QTmodelViewer = new QTModelViewer();
    QTmodelViewer->show();

    return a.exec();
}