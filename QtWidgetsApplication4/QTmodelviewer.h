#pragma once
#include <QMainWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DWindow>
#include <QKeyEvent>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QMesh>
#include <QUrl>
#include <QSlider>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "algorithm.h"

namespace Ui {
    class QTModelViewer;
}

class QTModelViewer : public QMainWindow
{
    Q_OBJECT

public:
    QString old_iterations, old_lambda , old_decimateRate;
    explicit QTModelViewer(QWidget* parent = nullptr);
    ~QTModelViewer();
protected:
    bool eventFilter(QObject* obj, QEvent* event);
signals:
    void KeyPressed();
private slots:
    void check_text_int(const QString& new_val);
    void check_text_double1(const QString& new_val);
    void check_text_double2(const QString& new_val);
    void on_actionOpen_File_triggered();
private:
    Ui::QTModelViewer* ui;
    QSlider* createSlider();
    QPushButton* createButton_laplacianSmoothing();
    QPushButton* createButton_decimateMesh_Official();
    QPushButton* createButton_decimateMesh_Manual();
    QLineEdit* createEdit(QString defaultVal);
    Algorithm* algorithm;

    QLineEdit* iterations, * lambda, * decimateRate;

    QString currentFile = "";
};