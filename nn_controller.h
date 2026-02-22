#ifndef NN_CONTROLLER
#define NN_CONTROLLER
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace cv::dnn::dnn4_v20220524;
using namespace std;

class NNController {
public:
    NNController ();
    void init();
    void CatMatch();
    void DogMatch();
    bool categorise(std::string imgFilename);
    void NMS();

private:
    const int NN_CAT_CODE_CAT = 15; // eliminating hard coded values
    const int NN_CAT_CODE_DOG = 16;

    vector<string> classes;
    Net net;
};

#endif // NN_CONTROLLER