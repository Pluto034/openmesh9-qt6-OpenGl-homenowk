#include "stdafx.h"
#include "QTmodelviewer.h"
#include "ui_QTmodelviewer.h"
#include "algorithm.h"
QTModelViewer::QTModelViewer(QWidget* parent) :  QMainWindow(parent),ui(new Ui::QTModelViewer){
    ui->setupUi(this);
    setFocusPolicy(Qt::TabFocus);

    //central widget
    QWidget* central = new QWidget();
    setCentralWidget(central);

    //algorithm
    Qt3DExtras::Qt3DWindow* view = new Qt3DExtras::Qt3DWindow();
    QWidget* algorithmWidget = QWidget::createWindowContainer(view);
    algorithm = new Algorithm(view);
    view->installEventFilter(this);


    QHBoxLayout* algorithmLayout = new QHBoxLayout;
    algorithmLayout->addWidget(algorithmWidget);


    QWidget* mainWidget = new QWidget();
    mainWidget->setLayout(algorithmLayout);

    QPushButton* actionButton1 = createButton_laplacianSmoothing();
    connect(actionButton1, &QPushButton::clicked, algorithm, &Algorithm::start_laplacianSmoothing);

    

    this->old_lambda = "0.2";
    this->old_num = "3";

    this->algorithm->setLambda(0.2);
    this->algorithm->setdecimateRateOF(0.1);
    this->algorithm->setIters(3);

    num = createEdit(old_num), lambda = createEdit(old_lambda);

    connect(lambda, &QLineEdit::textChanged, this, &QTModelViewer::check_text_double1);
    connect(num, &QLineEdit::textChanged, this, &QTModelViewer::check_text_int);
    


    QLabel* num_label = new QLabel(), * lambda_label = new QLabel()
        , * decimateRateOF_label= new QLabel();
    QHBoxLayout* num_box = new QHBoxLayout(), * lambda_box = new QHBoxLayout();

    num_label->setText("itor_num");
    lambda_label->setText("lambda");
   
    

    num_box->addWidget(num_label);
    lambda_box->addWidget(lambda_label);

    num_box->addWidget(num);
    lambda_box->addWidget(lambda);


    QVBoxLayout* topLayout = new QVBoxLayout;
    topLayout->addWidget(mainWidget);

    topLayout->addLayout(num_box);
    topLayout->addLayout(lambda_box);

    topLayout->addWidget(actionButton1);


    central->setLayout(topLayout);
}


QPushButton* QTModelViewer::createButton_laplacianSmoothing()
{
    QPushButton* button = new QPushButton();
    button->setText("Avg_Smooth");
    return button;
}



QLineEdit* QTModelViewer::createEdit(QString defaultVal) {
    QLineEdit* editor = new QLineEdit();
    editor->setText(defaultVal);
    return editor;
}

QTModelViewer::~QTModelViewer()
{
    delete ui;
}

bool QTModelViewer::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
    
    case QEvent::Enter:
    case QEvent::Leave:
    {
        algorithm->MouseControls(nullptr, event->type());
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent == nullptr) {
            break;
        }
        algorithm->MouseControls(mouseEvent, event->type());
        break;
    }
    default:
        qDebug() << "Unsupported events:" << event->type() << '\n';
        break;
    }

    return QObject::eventFilter(obj, event);
}

void QTModelViewer::check_text_int(const QString& new_val) {
    QLineEdit* editor = dynamic_cast<QLineEdit*>(sender());
    if (editor == nullptr) {
        qDebug() << "不支持的发送者" << '\n';
        return;
    }
    bool is_ok;
    int value = new_val.toInt(&is_ok);
    if (!is_ok) {
        QString old_val;
        if (editor == num) old_val = old_num;
        else if (editor == lambda) old_val = old_lambda;
        editor->setText(old_val);
        return;
    }
    else {
        if (editor == num) {
            old_num = new_val;
            this->algorithm->setIters(value);
        }
        else if (editor == lambda) {
            old_lambda = new_val;
            this->algorithm->setLambda(static_cast<double>(value));
        }
    }
}

void QTModelViewer::check_text_double1(const QString& new_val) {
    QLineEdit* editor = dynamic_cast<QLineEdit*>(sender());
    if (editor == nullptr) {
        qDebug() << "不支持的发送者" << '\n';
        return;
    }
    bool is_ok = true;
    double value = new_val.toDouble(&is_ok);
    if (!is_ok) {
        QString old_val;
        if (editor == num ) old_val = old_num;
        else if (editor == lambda) old_val = old_lambda;
        editor->setText(old_val);
        return;
    }
    else {
        if (editor == num) {
            old_num = new_val;
            this->algorithm->setIters(static_cast<int>(value));
        }
        else if (editor == lambda) {
            old_lambda = new_val;
            this->algorithm->setLambda(value);
        }
    }
}


void QTModelViewer::on_actionOpen_File_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open");
    QFile file(filename);
    currentFile = filename;
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open " + file.errorString());
    }
    setWindowTitle(filename);


    algorithm->cur_source = filename.toStdString();
    algorithm->NewAlgorithm();
}