#ifndef CAM_CONTROLLER
#define CAM_CONTROLLER
#include <string>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <libcamera/libcamera.h>
#include <libcamera/controls.h>
#include <libcamera/property_ids.h>

using namespace std::chrono;

class CameraController {
public:
    CameraController ();
    void init();

    void TakePhotoToFile(std::string imgFilename);
    void ActivateCamera();
    void DeactivateCamera();

    void CameraRequestComplete(libcamera::Request *request);

private: // for the callback function CameraRequestComplete()
    std::shared_ptr<libcamera::Camera> m_camera;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_done = false;
    steady_clock::time_point m_start_time;

    std::string m_imgFilename;
};

#endif // CAM_CONTROLLER