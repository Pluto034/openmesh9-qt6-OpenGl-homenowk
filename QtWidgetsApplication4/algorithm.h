#pragma once

#include <QObject>
#include <QWidget>

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DCore/QTransform>
#include <Qt3DWindow>

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTorusMesh>

#include <QKeyEvent>
#include <QTransform>
#include <QMesh>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Eigen/Core>
#include <Eigen/Sparse>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Smoother/LaplaceSmootherT.hh>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

#include <mutex>
#include <cmath>
#include <set>
#include <unordered_set>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <iostream>

using MyMesh = OpenMesh::TriMesh_ArrayKernelT<>;

//typedef OpenMesh::Decimater::DecimaterT<MyMesh>  Decimater;
class Algorithm : public QObject
{
    Q_OBJECT
public:
    void setLambda(double new_val);
    void setIters(int new_val);
    void setdecimateRateOF(double new_val);
    explicit Algorithm(Qt3DExtras::Qt3DWindow*, QObject* parent = nullptr);

    std::string cur_source;
    void NewAlgorithm();
    void KeyControls(QKeyEvent* event);
    void MouseControls(QMouseEvent* event, QEvent::Type eventType);


public slots:
    void LightXChanged(int angle);
    void LightYChanged(int angle);
    void LightZChanged(int angle);
    void start_laplacianSmoothing(bool checked);
    void start_decimateMesh_Official(bool checked);
    void start_decimateMesh_Manual(bool checked);

private:
    // �����һ�ε�λ��
    QPoint last_pos;
    // ��һ�θ��µ�ʱ������Ƿ��ڱ߽���
    bool is_in_sence;
    // ��һ�θ��µ�ʱ���������Ƿ񱻰���
    bool is_in_press;
    Qt3DRender::QMesh* curMesh;

    Qt3DCore::QEntity* rootEntity;
    Qt3DCore::QEntity* model;
    Qt3DCore::QTransform* transform;
    Qt3DRender::QDirectionalLight* light;

    void StartAlgorithm();
    void Camera(Qt3DExtras::Qt3DWindow* view);
    void SetupMesh(Qt3DRender::QMesh* mesh);
    void SetupTransform();
    void SetupMaterial();
    void SetupLighting();

    void scale(MyMesh& mesh);

    double lambda;
    int iters;
    double decimateRateOF;
};