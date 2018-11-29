#pragma once
#include <opencv2/opencv.hpp>


namespace cv {

Rect selectROI(InputArray img, bool showCrosshair, bool fromCenter);

Rect selectROI(const String& windowName, InputArray img, bool showCrosshair, bool fromCenter);

void selectROIs(const String& windowName, InputArray img,
                             std::vector<Rect>& boundingBox, bool showCrosshair, bool fromCenter);

}
