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

    void StartCameraLoop();
    void StopCameraLoop();
    void TakePhotoToFile(std::string imgFilename);

    void ActivateCamera();

    void CameraRequestComplete(libcamera::Request *request);

private: // for the callback function CameraRequestComplete()
    std::shared_ptr<libcamera::Camera> m_camera;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_done = false;
    steady_clock::time_point m_start_time;
    
    bool m_isTakeOnlySinglePhotoMode = false;
    bool m_isMonitoringMode = false;  // for continuous monitoring mode

    std::string m_imgFilename;
};

#endif // CAM_CONTROLLER