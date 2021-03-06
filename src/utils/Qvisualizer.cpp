#include "include/utils/Qvisualizer.h"

QGLVisualizer::QGLVisualizer(void) : objectPose(Mat34::Identity()), cameraPose(Mat34::Identity()) {
    objectPose(0, 3) = 2.0;
}

/// Destruction
QGLVisualizer::~QGLVisualizer(void) {}

//}

/// draw objects
void QGLVisualizer::draw() {
    glPushMatrix();
    glBegin(GL_POINTS);
    std::vector<Point3D> points = pointCloud.getPoints3D();
    for (Point3D i : points) {
        glColor3f(i.red / 255.0f, i.green / 255.0f, i.blue / 255.0f);
        glVertex3f(i.position(0), -i.position(1), -i.position(2));
    }
    glEnd();

    // Drawing normals
    double normalScaleFactor = 5.0f;
    for (auto plane : planes){
        Vector3d point = plane.getCentralPoint().position;
        Vector3d normalVec = plane.getPlaneNormalVec() / normalScaleFactor;
        glLineWidth(2);
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(point(0), -point(1), -point(2));
        glVertex3f(point(0) + normalVec(0), -point(1) + normalVec(1), -point(2) + normalVec(2));
        glEnd();
    }

    glPopMatrix();
}

/// draw objects
void QGLVisualizer::animate() {
}

/// initialize visualizer
void QGLVisualizer::init() {
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    camera()->setZNearCoefficient(0.00001f);
    camera()->setZClippingCoefficient(100.0);

    setBackgroundColor(QColor(100, 100, 100));

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    startAnimation();
}


/// Updates cloud
void QGLVisualizer::updateCloud(cv::Mat RGB, cv::Mat D) {
    depth2cloud(D, RGB);
}

void QGLVisualizer::getPoint(unsigned int u, unsigned int v, double depth, Eigen::Vector3d &point3D) {
    Eigen::Vector3d point(u, v, 1);
    point3D = depth * PHCPModel * point;
}

void QGLVisualizer::setPHCPModel(Eigen::Matrix<double, 3, 3> model) {
    pointCloud.setPHCPModel(model);
}

/// Convert disparity image to point cloud
void QGLVisualizer::depth2cloud(cv::Mat &depthImage, cv::Mat RGB) {
    pointCloud.depth2cloud(depthImage, RGB);
}

void QGLVisualizer::updateCloud(Registration *registration, Frame *undistorted,
                                Frame *registered, const PosOrient &posOrient) {
    pointCloud.clear();
    for (int r = 0; r < undistorted->height; ++r) {
        for (int c = 0; c < undistorted->width; ++c) {
            float x, y, z, color;
            registration->getPointXYZRGB(undistorted, registered, r, c, x, y, z, color);
            const uint8_t *p = reinterpret_cast<uint8_t *>(&color);
            if (!isnanf(z)) {
                Point3D point3D(-x, -y, -z, p[2], p[1], p[0]);
                utils::rotatePoint(point3D, posOrient);
                pointCloud.push_back(point3D);
            }
        }
    }
}

void QGLVisualizer::keyPressEvent(QKeyEvent *event) {
    int key = event->key();

    switch (key) {
        case Qt::Key_Q:
            setProgramFinished(true);
            close();
            break;
        default:
            QGLViewer::keyPressEvent(event);
    }
}

const PointCloud &QGLVisualizer::getPointCloud() const {
    return pointCloud;
}

bool QGLVisualizer::isProgramFinished() const {
    return programFinished;
}

void QGLVisualizer::setProgramFinished(bool programFinished) {
    QGLVisualizer::programFinished = programFinished;
}

void QGLVisualizer::updatePlanes(std::vector<Plane> &planes){
    this->planes = planes;
}

void QGLVisualizer::updateCloud(Mat RGB, Mat D, const PosOrient &posOrient) {
    globalDatasetPointCloud.depth2cloud(D, RGB, posOrient);
}

const PointCloud &QGLVisualizer::getGlobalDatasetPointCloud() const {
    return globalDatasetPointCloud;
}
