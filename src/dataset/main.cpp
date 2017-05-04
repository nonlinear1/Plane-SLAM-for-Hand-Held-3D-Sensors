//
//  main.cpp
//  Projekt Magisterski
//
//  Created by Marcin Stramowski on 15.12.2016.
//  Copyright © 2016 Marcin Stramowski. All rights reserved.
//

#include "include/dataset/main.h"

int main(int argc, char **argv) {

    //some testing stuff
    std::vector<Plane> planesVec;
    Eigen::Vector3f v1, v2, v3, v4, v5;
    v1 << cos(30.0*M_PI/180.0), sin(30.0*M_PI/180.0), 0;
    v2 << 1, 0, 0;
    v3 << 1, -1, 0;
    v4 << -1, -1, 0;
    v5 << -1, 0, 0;
    Plane p1(v1, 0), p2(v2, 5), p3(v3, 0), p4(v4, 0), p5(v5, 0);
    planesVec.push_back(p1);
    planesVec.push_back(p2);
    planesVec.push_back(p3);
    planesVec.push_back(p4);
    planesVec.push_back(p5);
    std::vector<Cluster> clustersVec;
    std::vector<std::unordered_set<int>> vecEachClusterPlanes;
    Clustering::computeClusters(planesVec, clustersVec);
    Clustering::getClustersAfterThreshold(46.0f, planesVec, vecEachClusterPlanes);

    for(auto i : clustersVec){
        std::cout<<i.getFirstLinkIndex() << " "<< i.getSecondLinkIndex() << " "<<i.getDistanceBetweenLinks()<<std::endl;
    }

    std::cout<<std::endl;
    for(auto i : vecEachClusterPlanes){
        for(auto j : i){
            std::cout<<j<<" ";
        }
        std::cout<<std::endl;
    }




    /////////////////////////////////////

    QApplication application(argc, argv);
    glutInit(&argc, argv);

    QGLVisualizer visualizer;
    visualizer.setWindowTitle("Dataset viewer");
    visualizer.setPHCPModel(PHCP_MODEL);
    visualizer.show();

    ImageLoader imageLoader(50);

    vector<Plane> planeVectorPreviousFrame;
    vector<Plane> planeVectorCurrentFrame;
    vector<pair<Plane, Plane>> similarPlanes;

    const int AREA_SIZE = 21; // odd number
    const int NUMBER_OF_POINTS = 10;
    if (AREA_SIZE % 2 == 0) throw runtime_error("AREA_SIZE needs to be odd number");

    ImagePair imagePair1 = imageLoader.getNextPair();
    utils::fillPlaneVector(NUMBER_OF_POINTS, AREA_SIZE, imagePair1, planeVectorPreviousFrame);

    vector<vector<Plane>> clusteredPLanes;
    Clustering::getClusteredPlaneGroup(planeVectorPreviousFrame, clusteredPLanes);
    int i = 0;
    for (auto singleCluster : clusteredPLanes){
        ++i;
        for(auto planeInCluster : singleCluster){
            putText(imagePair1.getRgb(), to_string(i), Point(planeInCluster.getImageCoords().getCenterX(), planeInCluster.getImageCoords().getCenterY()),FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
        }
    }



    visualizer.updateCloud(imagePair1.getRgb(), imagePair1.getDepth());
    imshow("First", imagePair1.getRgb());


    return application.exec();
    return 0;
}
