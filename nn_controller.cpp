#include "nn_controller.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <sstream>

using namespace cv;
using namespace cv::dnn::dnn4_v20220524;
using namespace std;

// Class names from COCO dataset (only showing relevant ones; load full from file)
vector<string> loadClasses(const string& classFile) {
    vector<string> classes;
    ifstream ifs(classFile.c_str());
    string line;
    while (getline(ifs, line)) {
        classes.push_back(line);
    }
    return classes;
}

NNController::NNController() {
}

void NNController::init() {
}

void NNController::categorise(std::string imgFilename) {
    std::cout << "Categorising image: " << imgFilename << std::endl;

    string cfgFile = "yolov3.cfg";
    string weightsFile = "yolov3.weights";
    string classesFile = "coco.names.txt";
    string outputPath = "output.jpg";

    // Load categorisation classes from coco.names.txt
    vector<string> classes = loadClasses(classesFile);
    cout << "Loaded " << classes.size() << " classes." << endl;
    // for (size_t i = 0; i < classes.size(); i++)
    // {
    //     cout << "Class " << i << ": " << classes[i] << endl;
    // }
    // for debugging


    // Load input image
    Mat img = imread(imgFilename);
    if (img.empty()) {
        cout << "Could not load image: " << imgFilename << endl;
        return;
    }

    // Load YOLO network
    Net net = readNetFromDarknet(cfgFile, weightsFile);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    // From input image create a blob for YOLO (YOLO input size: 416x416)
    Mat blob;
    blobFromImage(img, blob, 1.0 / 255.0, Size(416, 416), Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    // Forward pass
    vector<Mat> outs;
    vector<String> outNames = net.getUnconnectedOutLayersNames();
    net.forward(outs, outNames);  // This call performs the forward pass

    // NMS parameters
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
    double confThreshold = 0.2; // was 0.5
    double nmsThreshold = 0.4;

    // Process detections
    bool isDrawAllConfidenceBoxes = false; // for debugging
    for (size_t i = 0; i < outs.size(); ++i) {
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

            if (confidence > confThreshold) {
                int centerX = (int)(data[0] * img.cols);
                int centerY = (int)(data[1] * img.rows);
                int width = (int)(data[2] * img.cols);
                int height = (int)(data[3] * img.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));

                cout << "Detected " <<  classes[classIdPoint.x] << " with confidence " << confidence << endl;

                if (isDrawAllConfidenceBoxes) {
                    // Draw all boxes before NMS for debugging
                    rectangle(img, Rect(left, top, width, height), Scalar(0, 0, 255), 1);
                    string label = classes[classIdPoint.x] + ": " + to_string(confidence).substr(0, 4);
                    putText(img, label, Point(left, top - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);
                }
            }
        }
    }

    cout << "Number of raw detections before NMS: " << classIds.size() << endl;
    cout << "Confidence threshold: " << confThreshold << endl;

    // Apply NMS (Non-Maximum Suppression): Suppress weak overlapping boxes
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // Check for cat (15) or dog (16). Zero indexing, cat 16th item, so index is 15.
    bool hasCatOrDog = false;
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        int cls = classIds[idx];
        if (cls == NN_CAT_CODE_CAT || cls == NN_CAT_CODE_DOG) {
            hasCatOrDog = true;
            Rect box = boxes[idx];
            float conf = confidences[idx];

            // Draw bounding box and label
            Scalar color = (cls == NN_CAT_CODE_CAT) ? Scalar(0, 255, 0) : Scalar(255, 0, 0);  // Green for cat, blue for dog
            rectangle(img, box, color, 2);
            string label = classes[cls] + ": " + to_string(conf).substr(0, 4);
            putText(img, label, Point(box.x, box.y - 5), FONT_HERSHEY_SIMPLEX, 0.5, color, 2);
        }
    }

    // Save output
    imwrite(outputPath, img);

    // Output result
    if (hasCatOrDog) {
        cout << "Cat or dog detected in the image!" << endl;
    } else {
        cout << "No cat or dog detected in the image." << endl;
    }

    cout << "Annotated image saved as: " << outputPath << endl;
}