//
// Created by stramek on 25.08.17.
//

#include "include/planeG2O/GlobalG2oMap.h"

Eigen::Quaterniond GlobalG2oMap::normAndDToQuat(double d, Eigen::Vector3d norm) {
    Eigen::Quaterniond res;
    norm.normalize();
    res.x() = norm[0];
    res.y() = norm[1];
    res.z() = norm[2];
    res.w() = -d;

    g2o::VertexPlaneQuat::normalizeAndUnify(res);
    return res;
}

GlobalG2oMap::GlobalG2oMap() {}

void GlobalG2oMap::initializeFirstFrame(vector<Plane> &planes) {
    positionNumber = 0;

    g2o::BlockSolverX::LinearSolverType *linearSolverMin = new g2o::LinearSolverPCG<g2o::BlockSolverX::PoseMatrixType>();
    g2o::BlockSolverX *solverMin = new g2o::BlockSolverX(linearSolverMin);
    g2o::OptimizationAlgorithmLevenberg *algorithmMin = new g2o::OptimizationAlgorithmLevenberg(solverMin);
    optimizerMin.setAlgorithm(algorithmMin);

    //set init camera pose
    g2o::VertexSE3Quat *curV = new g2o::VertexSE3Quat();
    Vector3d trans(0.0, 0.0, 0.0);
    Quaterniond q;
    q.setIdentity();
    g2o::SE3Quat poseSE3Quat(q, trans);
    curV->setEstimate(poseSE3Quat);
    cout<<"Adding VertexSE3Quat id = " << CAMERA_POS_INDEXES_SHIFT + positionNumber <<endl;
    curV->setId(CAMERA_POS_INDEXES_SHIFT + positionNumber);
    curV->setFixed(true);

    optimizerMin.addVertex(curV);

    for (Plane &plane : planes) {

        pair<Plane, bool> status = GlobalMap::getInstance().addPlaneToMap(plane, lastPosOrient);

        if (status.second) {
            //add planes to graph
            g2o::VertexPlaneQuat *curV2 = new g2o::VertexPlaneQuat();
            curV2->setEstimate(normAndDToQuat(plane.getD(), plane.getPlaneNormalVec()));
            cout<<"Adding VertexPlaneQuat id = " << status.first <<endl;
            curV2->setId((int) status.first.getId());
            optimizerMin.addVertex(curV2);

            //add edge to graph
            g2o::EdgeSE3Plane *curEdge = new g2o::EdgeSE3Plane();
            curEdge->setVertex(0, optimizerMin.vertex(CAMERA_POS_INDEXES_SHIFT + positionNumber));
            curEdge->setVertex(1, optimizerMin.vertex((int) status.first.getId()));

            curEdge->setMeasurement(normAndDToQuat(plane.getD(), plane.getPlaneNormalVec()));

            curEdge->setInformation(Eigen::Matrix<double, 3, 3>::Identity());

            optimizerMin.addEdge(curEdge);
        }
    }
}

void GlobalG2oMap::addNextFramePlanes(vector<Plane> &planes) {
    positionNumber++; // increment camera position number

    //set init camera pose
    g2o::VertexSE3Quat *curV = new g2o::VertexSE3Quat();
    Vector3d trans(lastPosOrient.getPosition());
    Quaterniond q(lastPosOrient.getQuaternion());
    g2o::SE3Quat poseSE3Quat(q, trans);
    curV->setEstimate(poseSE3Quat);
    cout<<"Adding VertexSE3Quat id = " << CAMERA_POS_INDEXES_SHIFT + positionNumber <<endl;
    curV->setId(CAMERA_POS_INDEXES_SHIFT + positionNumber);

    optimizerMin.addVertex(curV);

    for (auto &plane : planes) {
        pair<Plane, bool> status = GlobalMap::getInstance().addPlaneToMap(plane, lastPosOrient);
        if (status.second) {
            g2o::VertexPlaneQuat *curV1 = new g2o::VertexPlaneQuat();
            curV1->setEstimate(normAndDToQuat(status.first.getD(), status.first.getPlaneNormalVec()));
            cout<<"Adding VertexPlaneQuat id = " << status.first <<endl;
            curV1->setId((int) status.first.getId());
            optimizerMin.addVertex(curV1);
        }

        //add edge to graph
        g2o::EdgeSE3Plane *curEdge = new g2o::EdgeSE3Plane();
        curEdge->setVertex(0, optimizerMin.vertex(CAMERA_POS_INDEXES_SHIFT + positionNumber));
        curEdge->setVertex(1, optimizerMin.vertex((int) status.first.getId()));

        curEdge->setMeasurement(normAndDToQuat(plane.getD(), plane.getPlaneNormalVec()));

        curEdge->setInformation(Eigen::Matrix<double, 3, 3>::Identity());

        optimizerMin.addEdge(curEdge);
    }

    optimizerMin.setVerbose(false);
    optimizerMin.initializeOptimization();
    optimizerMin.optimize(1000);

    g2o::VertexSE3Quat *curPoseVert = static_cast<g2o::VertexSE3Quat *>(optimizerMin.vertex(
            CAMERA_POS_INDEXES_SHIFT + positionNumber));
    g2o::Vector7d poseVect = curPoseVert->estimate().toVector();
    lastPosOrient.setPosOrient(poseVect);
    cout << endl << "Setting new posOrient to..." << endl;
    lastPosOrient.print();

    for (int i = 0; i < GlobalMap::getInstance().getCurrentId(); ++i) {
        g2o::VertexPlaneQuat *curPlaneVert = static_cast<g2o::VertexPlaneQuat *>(optimizerMin.vertex(i));
        Eigen::Quaterniond res = curPlaneVert->estimate();
        Plane plane(-res.w(), Eigen::Vector3d(res.x(), res.y(), res.z()));
        plane.setId(curPlaneVert->id());
        GlobalMap::getInstance().updatePlane(plane);
    }
    optimizerMin.save("output.txt");
}

const PosOrient &GlobalG2oMap::getLastPosOrient() const {
    return lastPosOrient;
}
