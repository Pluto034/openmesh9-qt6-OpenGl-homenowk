#include "stdafx.h"
#include "scene.h"
#include "QTmodelviewer.h"
#include <vector>
#include <algorithm>




Scene::Scene(Qt3DExtras::Qt3DWindow* view, QObject* parent) : QObject(parent)
{
	rootEntity = new Qt3DCore::QEntity();
	view->setRootEntity(rootEntity);
	view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x808080)));
	Camera(view);
	StartScene();

}



void Scene::Camera(Qt3DExtras::Qt3DWindow* view)
{
	Qt3DRender::QCamera* camera = view->camera();
	camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	camera->setPosition(QVector3D(0, 0, 40));
	camera->setViewCenter(QVector3D(0, 0, 0));
}

void Scene::StartScene()
{
	model = new Qt3DCore::QEntity(rootEntity);

	curMesh = nullptr;

	SetupTransform();
	SetupMaterial();
	SetupLighting();

}


void Scene::NewScene()
{
	{
		MyMesh mesh;
		// ���ļ��ж�ȡģ��
		if (!OpenMesh::IO::read_mesh(mesh, cur_source)) {
			qDebug() << "�޷���ȡģ���ļ�" << '\n';
			return;
		}
		scale(mesh);
		// �����д���ļ�
		if (!OpenMesh::IO::write_mesh(mesh, cur_source + ".obj")) {
			qDebug() << "�޷�д��ƽ�����ģ���ļ�" << '\n';
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

void Scene::SetupMesh(Qt3DRender::QMesh* mesh)
{
	model->addComponent(mesh);
	curMesh = (Qt3DRender::QMesh*)mesh;
}

void Scene::SetupTransform()
{
	transform = new Qt3DCore::QTransform(model);
	transform->setScale3D(QVector3D(1, 1, 1));
	transform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 0));
	model->addComponent(transform);
}

void Scene::SetupMaterial()
{
	Qt3DRender::QMaterial* material = new Qt3DExtras::QPhongMaterial(model);
	model->addComponent(material);
}

void Scene::SetupLighting()
{
	Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
	light = new Qt3DRender::QDirectionalLight(lightEntity);
	light->setColor("white");
	light->setIntensity(1);
	light->setWorldDirection(QVector3D(0, 0, -1));

	lightEntity->addComponent(light);
}

void Scene::KeyControls(QKeyEvent* event) {
	if (event->modifiers().testFlag(Qt::ControlModifier)) {
		if (event->key() == Qt::Key_Up) {
			transform->setRotationX(transform->rotationX() - 6);
		}
		if (event->key() == Qt::Key_Down) {
			transform->setRotationX(transform->rotationX() + 6);
		}
		if (event->key() == Qt::Key_Left) {
			transform->setRotationY(transform->rotationY() - 6);
		}
		if (event->key() == Qt::Key_Right) {
			transform->setRotationY(transform->rotationY() + 6);
		}
	}
	else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
		if (event->key() == Qt::Key_Up) {
			transform->setTranslation(QVector3D(transform->translation().x(), transform->translation().y(), transform->translation().z() - 1));
		}
		if (event->key() == Qt::Key_Down) {
			transform->setTranslation(QVector3D(transform->translation().x(), transform->translation().y(), transform->translation().z() + 1));
		}
	}
	else {
		if (event->key() == Qt::Key_Up) {
			transform->setTranslation(QVector3D(transform->translation().x(), transform->translation().y() + 1, transform->translation().z()));
		}
		if (event->key() == Qt::Key_Down) {
			transform->setTranslation(QVector3D(transform->translation().x(), transform->translation().y() - 1, transform->translation().z()));
		}
		if (event->key() == Qt::Key_Left) {
			transform->setTranslation(QVector3D(transform->translation().x() - 1, transform->translation().y(), transform->translation().z()));
		}
		if (event->key() == Qt::Key_Right) {
			transform->setTranslation(QVector3D(transform->translation().x() + 1, transform->translation().y(), transform->translation().z()));
		}
	}
}

void Scene::MouseControls(QMouseEvent* event, QEvent::Type eventType) {
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

void Scene::LightXChanged(int angle)
{
	light->setWorldDirection(QVector3D(angle, light->worldDirection().y(), light->worldDirection().z()));
}

void Scene::LightYChanged(int angle)
{
	light->setWorldDirection(QVector3D(light->worldDirection().x(), angle, light->worldDirection().z()));
}

void Scene::LightZChanged(int angle)
{
	light->setWorldDirection(QVector3D(light->worldDirection().x(), light->worldDirection().y(), angle));
}


void Scene::scale(MyMesh& mesh) {
	// 2. ����ģ�͵�����λ��
	MyMesh::Point center(0.0, 0.0, 0.0);
	for (const auto& v : mesh.vertices()) {
		center += mesh.point(v);
	}
	center /= mesh.n_vertices();

	// 3. ��ģ��ƽ�Ƶ�����λ��
	for (auto& v : mesh.vertices()) {
		mesh.set_point(v, mesh.point(v) - center);
	}

	// 4. ����ģ�͵İ�Χ�д�С���������ߴ�ָ�꣩������ʹ�ð�Χ�еĶԽ��߳�����Ϊ�ο�
	float diagonal_length = 0.0f;
	for (const auto& v : mesh.vertices()) {
		float distance = (mesh.point(v) - MyMesh::Point(0.0, 0.0, 0.0)).length();
		if (distance > diagonal_length) {
			diagonal_length = distance;
		}
	}

	// 5. ����ģ�ͣ�ʹ���С�ʺ�����ľ���
	float target_distance = 10.0f; // Ŀ�����
	float scale_factor = target_distance / diagonal_length;
	for (auto& v : mesh.vertices()) {
		mesh.set_point(v, mesh.point(v) * scale_factor);
	}
}


void decimateMesh_Official(MyMesh& mesh, float targetReduction)
{
	OpenMesh::Decimater::DecimaterT<MyMesh> decimater(mesh);
	OpenMesh::Decimater::ModQuadricT<MyMesh>::Handle hModQuadric;
	decimater.add(hModQuadric);

	size_t targetFaces = static_cast<size_t>(mesh.n_faces() * (targetReduction));
	decimater.initialize();
	int a = mesh.n_faces();
	std::cout << mesh.n_faces() << " " << targetFaces << "\n";
	decimater.decimate_to_faces(0, targetFaces);
	mesh.garbage_collection();
}


struct Vertex {
    MyMesh::Point point; // ��������
    bool isBoundary;     // �߽��־
};
std::mutex mutex;
struct Face {  // ��ṹ��
    std::vector<size_t> vertices; // ��������
};
float distanceSquared(const Vertex& v1, const Vertex& v2) {  // ������������֮��ľ����ƽ��
    MyMesh::Point diff = v1.point - v2.point;
    return diff.sqrnorm();
}
float triangleArea(const Vertex& v1, const Vertex& v2, const Vertex& v3) {   // ���������ε����
    MyMesh::Point ab = v2.point - v1.point;
    MyMesh::Point ac = v3.point - v1.point;
    return (ab % ac).norm() * 0.5f;
}
void decimateMesh_Manual(MyMesh& mesh, float targetReduction) {
	
	std::vector<Vertex> vertices(mesh.n_vertices());
	std::vector<Face> faces(mesh.n_faces());

	// �������㣬��䶥������
	for (size_t vIndex = 0; vIndex < mesh.n_vertices(); ++vIndex) {
		vertices[vIndex].point = mesh.point(MyMesh::VertexHandle(vIndex));
		// ��ʵ��Ӧ������Ҫ��� isBoundary ��־
		vertices[vIndex].isBoundary = false;
	}

	// �����棬���������
	for (size_t fIndex = 0; fIndex < mesh.n_faces(); ++fIndex) {
		Face& face = faces[fIndex];
		face.vertices.clear();
		for (auto v_it = mesh.fv_iter(MyMesh::FaceHandle(fIndex)); v_it.is_valid(); ++v_it) {
			face.vertices.push_back((*v_it).idx());
		}
	}

	size_t target_faces = mesh.n_faces() * targetReduction;
	// ѭ��ֱ���ﵽĿ������
	while (faces.size() > target_faces) {
		// �ҵ������С��������
		size_t min_area_index = 0;
		float min_area = triangleArea(vertices[faces[0].vertices[0]], vertices[faces[0].vertices[1]], vertices[faces[0].vertices[2]]);
		for (size_t i = 1; i < faces.size(); ++i) {
			float area = triangleArea(vertices[faces[i].vertices[0]], vertices[faces[i].vertices[1]], vertices[faces[i].vertices[2]]);
			if (area < min_area) {
				min_area = area;
				min_area_index = i;
			}
		}
		// �ҵ�����С������������ڵ�����������
		std::vector<size_t> adjacent_faces;
		for (size_t i = 0; i < faces.size(); ++i) {
			const Face& face = faces[i];
			if (face.vertices[0] == min_area_index || face.vertices[1] == min_area_index || face.vertices[2] == min_area_index) {
				adjacent_faces.push_back(i);
			}
		}
		// �Ƴ�����С������������ڵ�������
		for (size_t i = 0; i < adjacent_faces.size(); ++i) {
			size_t adj_face_index = adjacent_faces[i];
			faces.erase(faces.begin() + adj_face_index);
			// �������������ε�����������ɾ��������Խ��
			for (size_t j = i + 1; j < adjacent_faces.size(); ++j) {
				if (adjacent_faces[j] > adj_face_index) {
					adjacent_faces[j]--;
				}
			}
		}
		// �Ƴ���С���������
		faces.erase(faces.begin() + min_area_index);
		// ������������
		// �������
		mesh.clear();
		// ������Ӷ���
		std::vector<MyMesh::VertexHandle> added_vertices;
		for (const auto& vertex : vertices) {
			added_vertices.push_back(mesh.add_vertex(vertex.point));
		}
		//���������
		for (const auto& face : faces) {
			std::vector<MyMesh::VertexHandle> face_vertices;
			for (size_t vIndex : face.vertices) {
				face_vertices.push_back(added_vertices[vIndex]);
			}
			mesh.add_face(face_vertices);
		}
	}
}


void remesh(MyMesh& mesh, std::vector<std::vector<int>>& E, double eps = 1e-4);
void work(int begin, int end, double lambda, MyMesh& mesh, MyMesh& smoothedMesh, std::mutex& mutex, std::vector<std::vector<int>>& E);
void laplacianSmoothing(MyMesh& mesh, int iterations, double lambda) {
	int max_thread = 20;
	size_t n = mesh.n_vertices();
	std::vector<std::vector<int>> E(n);

	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
		MyMesh::VertexHandle vh = *v_it;
		for (auto vv_it = mesh.vv_iter(vh); vv_it.is_valid(); ++vv_it) {
			MyMesh::VertexHandle vh_to = *vv_it;
			E[vh.idx()].emplace_back(vh_to.idx());
		}
	}
	std::mutex mutex{};

	while (iterations--) {
		MyMesh smoothedMesh = mesh;
		int block_size = (n + max_thread - 1) / max_thread, block_num = (n + block_size - 1) / block_size;
		{
			std::vector<std::jthread> thread_pool;
			for (int i = 0; i < block_num; i++) {
				int begin = i * block_size;
				int end = (i + 1) * block_size;
				end = std::min(end, (int)n);
				thread_pool.emplace_back(work, begin, end, lambda, std::ref(mesh), std::ref(smoothedMesh), std::ref(mutex), std::ref(E));
			}
		}
		mesh = std::move(smoothedMesh);
		remesh(mesh, E, 1e-7);
		n = mesh.n_vertices();
	}
}




void remesh(
	MyMesh& mesh,
	std::vector<std::vector<int>>& E,
	double eps) {

	size_t sub_face = 0;
	for (auto face : mesh.all_faces()) {
		if (mesh.calc_face_area(face) < eps) sub_face++;
	}
	if (sub_face < 0.1 * mesh.n_faces()) return;

	OpenMesh::Decimater::DecimaterT<MyMesh> decimater(mesh);
	OpenMesh::Decimater::ModQuadricT<MyMesh>::Handle hModQuadric;
	decimater.add(hModQuadric);

	decimater.initialize();

	decimater.decimate_to_faces(0, mesh.n_faces() - sub_face);

	// Remove the deleted faces and vertices
	mesh.garbage_collection();

	E.resize(mesh.n_vertices(), std::vector<int>{});
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
		MyMesh::VertexHandle vh = *v_it;
		for (auto vv_it = mesh.vv_iter(vh); vv_it.is_valid(); ++vv_it) {
			MyMesh::VertexHandle vh_to = *vv_it;
			E[vh.idx()].emplace_back(vh_to.idx());
		}
	}
}

void work(int begin, int end, double lambda, MyMesh& mesh, MyMesh& smoothedMesh, std::mutex& mutex, std::vector<std::vector<int>>& E) {

	for (int i = begin; i < end; i++) {
		MyMesh::VertexHandle vh = mesh.vertex_handle(i);
		OpenMesh::DefaultTraits::Point origin_tmp = mesh.point(vh);
		Eigen::Vector3d sum(0, 0, 0), origin{ origin_tmp[0], origin_tmp[1], origin_tmp[2] };

		double totalWeight = lambda + E[vh.idx()].size();
		for (auto j : E[i]) {
			MyMesh::VertexHandle vh_to = mesh.vertex_handle(j);

			auto temp = mesh.point(vh_to);
			Eigen::Vector3d tmp{ temp[0], temp[1], temp[2] };

			sum += tmp;
		}

		auto temp = mesh.point(vh);
		sum += lambda * Eigen::Vector3d{ temp[0], temp[1], temp[2] };

		Eigen::Vector3d new_pos = sum / totalWeight;

		{
			//mutex.lock();
			MyMesh::VertexHandle vh_from = smoothedMesh.vertex_handle(vh.idx());
			smoothedMesh.set_point(vh_from, MyMesh::Point(new_pos[0], new_pos[1], new_pos[2]));
			//mutex.unlock();
		}
	}
}


void Scene::setLambda(double new_val) {
	lambda = new_val;
}
void Scene::setIters(int new_val) {
	iters = new_val;
}


void Scene::start_laplacianSmoothing(bool checked)
{
	MyMesh mesh;
	// ���ļ��ж�ȡģ��
	model->removeComponent(curMesh);
	if (!OpenMesh::IO::read_mesh(mesh, cur_source)) {
		qDebug() << "�޷���ȡģ���ļ�" << '\n';
		return;
	}
	laplacianSmoothing(mesh, iters, lambda);
	scale(mesh);
	// �����д���ļ�
	if (!OpenMesh::IO::write_mesh(mesh, "t1.obj")) {
		qDebug() << "�޷�д��ƽ�����ģ���ļ�" << '\n';
		return;
	}
	curMesh->setSource(QUrl::fromLocalFile(QString::fromStdString("t1.obj")));
	model->addComponent(curMesh);
}
void Scene::start_decimateMesh_Official(bool checked)
{
	MyMesh mesh;
	// ���ļ��ж�ȡģ��
	model->removeComponent(curMesh);
	if (!OpenMesh::IO::read_mesh(mesh, cur_source)) {
		qDebug() << "�޷���ȡģ���ļ�" << '\n';
		return;
	}
	// ִ�������
	decimateMesh_Official(mesh, 0.1f); // ����������Ҫ����90%����
	scale(mesh);
	// �����д���ļ�
	if (!OpenMesh::IO::write_mesh(mesh, "t2.obj")) {
		qDebug() << "�޷�д��ƽ�����ģ���ļ�" << '\n';
		return;
	}
	curMesh->setSource(QUrl::fromLocalFile(QString::fromStdString( "t2.obj")));
	model->addComponent(curMesh);
}
void Scene::start_decimateMesh_Manual(bool checked)
{
	MyMesh mesh;
	// ���ļ��ж�ȡģ��
	model->removeComponent(curMesh);
	if (!OpenMesh::IO::read_mesh(mesh, cur_source)) {
		qDebug() << "�޷���ȡģ���ļ�" << '\n';
		return;
	}
	// ִ�������
	decimateMesh_Manual(mesh, 0.9f); // ����������Ҫ����90%����
	scale(mesh);
	// �����д���ļ�
	if (!OpenMesh::IO::write_mesh(mesh,  "t3.obj"))  {
		qDebug() << "�޷�д��ƽ�����ģ���ļ�" << '\n';
		return;
	}

	curMesh->setSource(QUrl::fromLocalFile(QString::fromStdString( "t3.obj")));
	model->addComponent(curMesh);
}