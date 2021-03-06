//
// Created by stramek on 25.08.17.
//

#ifndef PROJEKTMAGISTERSKI_GLOBALG2OMAP_H
#define PROJEKTMAGISTERSKI_GLOBALG2OMAP_H


#include <include/models/Plane.h>
#include <include/demoG2o/main.h>
#include <g2o/solvers/pcg/linear_solver_pcg.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include "include/planeG2O/GlobalMap.h"
#include "include/models/PosOrient.h"

class GlobalG2oMap {
public:
    GlobalG2oMap();
    const PosOrient &getLastPosOrient() const;
    void addNewFrames(vector<Plane> &planes);
    void saveTrajectoryToFile();
    const vector<pair<Plane, Plane>> &getMatchedPlanes() const;

    void printLastPoseOrient();
private:
    bool initialized = false;
    void initializeFirstFrame(vector<Plane> &planes);
    void addNextFramePlanes(vector<Plane> &planes);
    int positionNumber;
    PosOrient lastPosOrient = PosOrient(Eigen::Vector3d(0, 0, 0), Eigen::Vector4d(0, 0, 0, 1));
    const int CAMERA_POS_INDEXES_SHIFT = 100000;
    Eigen::Quaterniond normAndDToQuat(double d, Eigen::Vector3d norm);
    vector<pair<Plane, Plane>> matchedPlanes;
    void loadFile();
    vector<PosOrient> pointSlamPosOrients;
};


#endif //PROJEKTMAGISTERSKI_GLOBALG2OMAP_H
