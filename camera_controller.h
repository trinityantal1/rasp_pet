#ifndef CAM_CONTROLLER
#define CAM_CONTROLLER

class CameraController {
public:
    CameraController ();
    void init();
    void TakePhoto();
    void ActivateCamera();
    void DeactivateCamera();
    void SaveToFile();

private:
    
};

#endif // CAM_CONTROLLER