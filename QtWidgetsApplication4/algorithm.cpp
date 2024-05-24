#include "stdafx.h"
#include "algorithm.h"
#include "QTmodelviewer.h"
#include <vector>
#include <algorithm>


Algorithm::Algorithm(Qt3DExtras::Qt3DWindow* view, QObject* parent) : QObject(parent)
{
	rootEntity = new Qt3DCore::QEntity();
	view->setRootEntity(rootEntity);
	view->defaultFrameGraph()->setClearColor(Qt::black);
	Camera(view);
	StartAlgorithm();
}
void Algorithm::Camera(Qt3DExtras::Qt3DWindow* view)
{
	Qt3DRender::QCamera* camera = view->camera();
	camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	camera->setPosition(QVector3D(0, 0, 40));
	camera->setViewCenter(QVector3D(0, 0, 0));
}
void Algorithm::StartAlgorithm()
{
	model = new Qt3DCore::QEntity(rootEntity);

	//curMesh = nullptr;

	SetupTransform();
	SetupMaterial();
	SetupLighting();

}
void Algorithm::NewAlgorithm()
{
	{
		MyMesh mesh;
		// 从文件中读取模型
		if (!OpenMesh::IO::read_mesh(mesh, cur_source)) {
			qDebug() << "无法读取模型文件" << '\n';
			return;
		}
		Tocenter(mesh);
		// 将结果写入文件
		if (!OpenMesh::IO::write_mesh(mesh, cur_source + ".obj")) {
			qDebug() << "无法写入平滑后的模型文件" << '\n';
			return;
		}
	}

	delete model;
	model = new Qt3DCore::QEntity(rootEntity);

	Qt3DRender::QMesh* mesh = new Qt3DRender::QMesh();
	QUrl data = QUrl::fromLocalFile(QString::fromStdString(cur_source + ".obj"));
	mesh->setSource(data);

	SetupMesh(mesh);
	SetupTransform();
	SetupMaterial();
	SetupLighting();
}
void Algorithm::SetupMesh(Qt3DRender::QMesh* mesh)
{
	model->addComponent(mesh);
	curMesh = (Qt3DRender::QMesh*)mesh;
}
void Algorithm::SetupTransform()
{
	transform = new Qt3DCore::QTransform(model);
	transform->setScale3D(QVector3D(1, 1, 1));
	transform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 0));
	model->addComponent(transform);
}
void Algorithm::SetupMaterial()
{
	Qt3DRender::QMaterial* material = new Qt3DExtras::QPhongMaterial(model);
	model->addComponent(material);
}
void Algorithm::SetupLighting()
{
	Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
	light = new Qt3DRender::QDirectionalLight(lightEntity);
	light->setColor("white");

	light->setIntensity(0.6f);
	light->setWorldDirection(QVector3D(0, 0, -1));

	lightEntity->addComponent(light);
}
void Algorithm::MouseControls(QMouseEvent* event, QEvent::Type eventType) {
	switch (eventType)
	{
	case QEvent::Enter:
		this->is_in_sence = true;
		return;
	case QEvent::Leave:
		this->is_in_sence = false;
		return;
	case QEvent::MouseButtonPress:
		this->is_in_press = true;
		return;
	case QEvent::MouseButtonRelease:
		this->is_in_press = false;
		return;
	case QEvent::MouseMove:
	{
		auto cur_pos = event->pos();
		if (this->is_in_sence and this->is_in_press) {
			auto [cur_x, cur_y] = cur_pos;

			double d_x = cur_x - last_pos.x();
			double d_y = cur_y - last_pos.y();

			d_x *= 6;
			d_y *= 6;

			qDebug() << d_x << ' ' << d_y << '\n';

			transform->setRotationX(transform->rotationX() + d_x);
			transform->setRotationY(transform->rotationY() + d_y);

		}
		this->last_pos = cur_pos;
	}
	return;
	default:
		break;
	}
}
void Algorithm::Tocenter(MyMesh& mesh) {
	// 2. 计算模型的中心位置
	MyMesh::Point center(0.0, 0.0, 0.0);
	for (const auto& v : mesh.vertices()) {
		center += mesh.point(v);
	}
	center /= mesh.n_vertices();

	// 3. 将模型平移到中心位置
	for (auto& v : mesh.vertices()) {
		mesh.set_point(v, mesh.point(v) - center);
	}

	// 4. 计算模型的包围盒大小（或其他尺寸指标），这里使用包围盒的对角线长度作为参考
	float diagonal_length = 0.0f;
	for (const auto& v : mesh.vertices()) {
		float distance = (mesh.point(v) - MyMesh::Point(0.0, 0.0, 0.0)).length();
		if (distance > diagonal_length) {
			diagonal_length = distance;
		}
	}

	// 5. 缩放模型，使其大小适合所需的距离
	float target_distance = 10.0f; // 目标距离
	float scale_factor = target_distance / diagonal_length;
	for (auto& v : mesh.vertices()) {
		mesh.set_point(v, mesh.point(v) * scale_factor);
	}
}





void Smooth(MyMesh& mesh, int num, double lambda) {
	int n = mesh.n_vertices(); // 获取网格顶点的数量
	std::vector<std::vector<int>> g(n); // 初始化一个邻接表g，用于存储每个顶点的邻居顶点索引

	// 遍历网格的所有顶点，填充邻接表
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
		MyMesh::VertexHandle vh = *v_it;
		for (auto vv_it = mesh.vv_iter(vh); vv_it.is_valid(); ++vv_it) {
			MyMesh::VertexHandle vh_to = *vv_it;
			g[vh.idx()].emplace_back(vh_to.idx());
		}
	}
	// 进行迭代平滑处理
	while (num--) {
		MyMesh smoothedMesh = mesh; // 创建一个平滑后的网格副本
		{
				for (int i = 0; i < n; i++) { // 所有点
					MyMesh::VertexHandle vh = mesh.vertex_handle(i); // 获取当前顶点的句柄

					// 获取当前顶点的原始位置
					OpenMesh::DefaultTraits::Point old_tmp = mesh.point(vh);
					Eigen::Vector3d old_pos{ old_tmp[0], old_tmp[1], old_tmp[2] };

					// 初始化用于计算新位置的变量
					Eigen::Vector3d sum(0, 0, 0);  
					//遍历当前顶点的所有邻居顶点，将它们的位置累加到sum中

					double totalWeight = g[vh.idx()].size() * (1) ; // 计算权重总和

					// 遍历当前顶点的所有邻居顶点，累加它们的位置
					for (auto j : g[i]) {
						MyMesh::VertexHandle vh_to = mesh.vertex_handle(j);
						auto temp = mesh.point(vh_to);
						Eigen::Vector3d tmp{ temp[0], temp[1], temp[2] };
						sum += tmp;
					}
					// 添加当前顶点的位置，权重为lambda
					sum += lambda * Eigen::Vector3d{ old_pos[0], old_pos[1], old_pos[2] };
					// 计算新位置
					Eigen::Vector3d new_pos = sum / totalWeight;

					new_pos =  old_pos + lambda *  (new_pos - old_pos);

					// 设置平滑后网格中对应顶点的新位置
					MyMesh::VertexHandle vh_from = smoothedMesh.vertex_handle(vh.idx());
					smoothedMesh.set_point(vh_from, MyMesh::Point(new_pos[0], new_pos[1], new_pos[2]));
				}
				mesh.garbage_collection();
		}
		mesh = smoothedMesh; // 更新网格为平滑后的网格
		mesh.garbage_collection();
		n = mesh.n_vertices(); // 更新顶点数量
		//size_t mm = mesh.n_faces();
	}
}


void Algorithm::setLambda(double new_val) {
	lambda = new_val;
}
void Algorithm::setIters(int new_val) {
	iters = new_val;
}
void Algorithm::setdecimateRateOF(double new_val) {
	decimateRateOF = new_val;
}


void Algorithm::start_laplacianSmoothing(bool checked)
{
	MyMesh mesh;
	OpenMesh::IO::read_mesh(mesh, cur_source);
	Smooth(mesh, iters, lambda);
	Tocenter(mesh);
	OpenMesh::IO::write_mesh(mesh, "t1.obj");
	curMesh->setSource(QUrl::fromLocalFile(QString::fromStdString("t1.obj")));
}
