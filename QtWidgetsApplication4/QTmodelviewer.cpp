#pragma execution_character_set("gbk") 
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

    //slider
    QSlider* xSlider = createSlider();
    QSlider* ySlider = createSlider();
    QSlider* zSlider = createSlider();
    zSlider->setValue(-1);

    connect(xSlider, &QSlider::valueChanged, algorithm, &Algorithm::LightXChanged);
    connect(ySlider, &QSlider::valueChanged, algorithm, &Algorithm::LightYChanged);
    connect(zSlider, &QSlider::valueChanged, algorithm, &Algorithm::LightZChanged);

    //layout
    QLabel* xLabel = new QLabel();
    QLabel* yLabel = new QLabel();
    QLabel* zLabel = new QLabel();
    xLabel->setText("X");
    yLabel->setText("Y");
    zLabel->setText("Z");

    QVBoxLayout* xLayout = new QVBoxLayout;
    QVBoxLayout* yLayout = new QVBoxLayout;
    QVBoxLayout* zLayout = new QVBoxLayout;
    xLayout->addWidget(xSlider);
    xLayout->addWidget(xLabel);
    yLayout->addWidget(ySlider);
    yLayout->addWidget(yLabel);
    zLayout->addWidget(zSlider);
    zLayout->addWidget(zLabel);
    QWidget* xWidget = new QWidget();
    QWidget* yWidget = new QWidget();
    QWidget* zWidget = new QWidget();
    xWidget->setLayout(xLayout);
    yWidget->setLayout(yLayout);
    zWidget->setLayout(zLayout);
    xWidget->setFixedWidth(50);
    yWidget->setFixedWidth(50);
    zWidget->setFixedWidth(50);

    QHBoxLayout* algorithmLayout = new QHBoxLayout;
    algorithmLayout->addWidget(algorithmWidget);
    algorithmLayout->addWidget(xWidget);
    algorithmLayout->addWidget(yWidget);
    algorithmLayout->addWidget(zWidget);
    QWidget* mainWidget = new QWidget();
    mainWidget->setLayout(algorithmLayout);

    QPushButton* actionButton1 = createButton_laplacianSmoothing();
    connect(actionButton1, &QPushButton::clicked, algorithm, &Algorithm::start_laplacianSmoothing);

    QPushButton* actionButton2 = createButton_decimateMesh_Official();
    connect(actionButton2, &QPushButton::clicked, algorithm, &Algorithm::start_decimateMesh_Official);

    QPushButton* actionButton3 = createButton_decimateMesh_Manual();
    connect(actionButton3, &QPushButton::clicked, algorithm, &Algorithm::start_decimateMesh_Manual);

    this->old_lambda = "0.5";
    this->old_iterations = "3";
    this->old_decimateRate = "0.1";

    this->algorithm->setLambda(0.5);
    this->algorithm->setdecimateRateOF(0.1);
    this->algorithm->setIters(3);

    iterations = createEdit(old_iterations), lambda = createEdit(old_lambda), decimateRate = createEdit(old_decimateRate);



    connect(decimateRate, &QLineEdit::textChanged, this, &QTModelViewer::check_text_double2);
    connect(lambda, &QLineEdit::textChanged, this, &QTModelViewer::check_text_double1);
    connect(iterations, &QLineEdit::textChanged, this, &QTModelViewer::check_text_int);
    


    QLabel* iterations_label = new QLabel(), * lambda_label = new QLabel()
        , * decimateRateOF_label= new QLabel();
    QHBoxLayout* iterations_box = new QHBoxLayout(), * lambda_box = new QHBoxLayout(), * decimateRate_box = new QHBoxLayout();

    iterations_label->setText("迭 代 次 数");
    lambda_label->setText("lambda 值");
    decimateRateOF_label->setText("简化程度");
    

    iterations_box->addWidget(iterations_label);
    lambda_box->addWidget(lambda_label);
    decimateRate_box->addWidget(decimateRateOF_label);

    iterations_box->addWidget(iterations);
    lambda_box->addWidget(lambda);
    decimateRate_box->addWidget(decimateRate);


    QVBoxLayout* topLayout = new QVBoxLayout;
    topLayout->addWidget(mainWidget);

    topLayout->addLayout(iterations_box);
    topLayout->addLayout(lambda_box);

    topLayout->addWidget(actionButton1);


    topLayout->addLayout(decimateRate_box);
    topLayout->addWidget(actionButton2);
    topLayout->addWidget(actionButton3);
    central->setLayout(topLayout);
}

QSlider* QTModelViewer::createSlider()
{
    QSlider* slider = new QSlider(Qt::Vertical);
    slider->setRange(-180, 180);
    slider->setSingleStep(1);
    slider->setPageStep(15);
    slider->setTickInterval(15);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}

QPushButton* QTModelViewer::createButton_laplacianSmoothing()
{
    QPushButton* button = new QPushButton();
    button->setText("平滑");
    return button;
}

QPushButton* QTModelViewer::createButton_decimateMesh_Official()
{
    QPushButton* button = new QPushButton();
    button->setText("简化-官方");
    return button;
}
QPushButton* QTModelViewer::createButton_decimateMesh_Manual()
{
    QPushButton* button = new QPushButton();
    button->setText("简化-手写");
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
    case QEvent::KeyPress:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        algorithm->KeyControls(keyEvent);
        break;
    }
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
            qDebug() << "Problems during event type conversion." << event->type() << '\n';
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

void QTModelViewer::check_text_int(const QString& new_val)
{
    QLineEdit* editor = dynamic_cast<QLineEdit*>(sender());
    if (editor == nullptr) {
        qDebug() << "不支持的发送者" << '\n';
        return;
    }

    QString old_val;
    bool is_ok = true;
    new_val.toInt(&is_ok);
    if (not is_ok) goto fail;
    else goto ok;

fail:
    if (editor == iterations) old_val = old_iterations;
    else if (editor == lambda) old_val = old_lambda;
    editor->setText(old_val);
    return;
ok:
    if (editor == iterations) old_iterations = new_val, this->algorithm->setIters(new_val.toInt());
    else if (editor == lambda) old_lambda = new_val, this->algorithm->setLambda(new_val.toDouble());
    
    return;
}

void QTModelViewer::check_text_double1(const QString& new_val)
{
    QLineEdit* editor = dynamic_cast<QLineEdit*>(sender());
    if (editor == nullptr) {
        qDebug() << "不支持的发送者" << '\n';
        return;
    }

    bool is_ok;
    is_ok = true;
    QString old_val;
    new_val.toDouble(&is_ok);
    if (not is_ok) goto fail;
    else goto ok;

fail:
    if (editor == iterations) old_val = old_iterations;
    else if (editor == lambda) old_val = old_lambda;
    
    editor->setText(old_val);
    return;
ok:
    if (editor == iterations) old_iterations = new_val, this->algorithm->setIters(new_val.toInt());
    else if (editor == lambda) old_lambda = new_val, this->algorithm->setLambda(new_val.toDouble());
    
    return;
}
void QTModelViewer::check_text_double2(const QString& new_val)
{
    QLineEdit* editor = dynamic_cast<QLineEdit*>(sender());
    if (editor == nullptr) {
        qDebug() << "不支持的发送者" << '\n';
        return;
    }

    bool is_ok;
    is_ok = true;
    QString old_val;
    new_val.toDouble(&is_ok);
    if (not is_ok) goto fail;
    else goto ok;

fail:
    if (editor == iterations) old_val = old_iterations;
    else if (editor == decimateRate) old_val = old_decimateRate;

    editor->setText(old_val);
    return;
ok:
    if (editor == iterations) old_iterations = new_val, this->algorithm->setIters(new_val.toInt());
    else if (editor == decimateRate) old_decimateRate = new_val, this->algorithm->setdecimateRateOF(new_val.toDouble());

    return;
}

void QTModelViewer::on_actionOpen_File_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    QFile file(filename);
    currentFile = filename;
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open " + file.errorString());
    }
    setWindowTitle(filename);


    algorithm->cur_source = filename.toStdString();
    algorithm->NewAlgorithm();
}