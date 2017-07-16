//
//  main.cpp
//  Projekt Magisterski
//
//  Created by Marcin Stramowski on 15.12.2016.
//  Copyright © 2016 Marcin Stramowski. All rights reserved.
//

#include "include/dataset/main.h"

int main(int argc, char **argv) {
    QApplication application(argc, argv);
    glutInit(&argc, argv);

    ImageLoader imageLoader(50);

    vector<Plane> planeVectorPreviousFrame;
    vector<Plane> planeVectorCurrentFrame;
    vector<pair<Plane, Plane>> similarPlanes;
    vector<PosOrient> idealSlamPositions;
    Mat previousRgbImage;

    const int AREA_SIZE = 35;
    const int NUMBER_OF_POINTS = 200;
    const bool visualize = true;

    utils::loadDatasetLines(idealSlamPositions);

    for (int i = 0; i < 5; ++i) {
        ImagePair currentFrame = imageLoader.getNextPair();

        make_unique<PlaneFillerBuilder>()
                ->withDataset(&currentFrame)
                ->withPlaneDetector(new PcaPlaneDetector())
                ->withAreaSize(AREA_SIZE)
                ->withNumberOfPoints(NUMBER_OF_POINTS)
                ->withPreviousPlanePercent(&planeVectorPreviousFrame, 0.5)
                ->build()
                ->fillVector(&planeVectorCurrentFrame);

        planeUtils::mergePlanes(planeVectorCurrentFrame);

        if (!planeVectorPreviousFrame.empty()) {
            similarPlanes = planeUtils::getSimilarPlanes(planeVectorPreviousFrame, planeVectorCurrentFrame);
            cout << "Frame " << i << "-" << i + 1 << " found: " << similarPlanes.size() << " similar planes." << endl;
            if (visualize)  {
                planeUtils::visualizeSimilarPlanes(similarPlanes, previousRgbImage, currentFrame.getRgb());
                waitKey();
            }
        }

        utils::movePlanesToPreviousVector(planeVectorPreviousFrame, planeVectorCurrentFrame);
        if (visualize) previousRgbImage = currentFrame.getRgb().clone();
    }

    return application.exec();
}
