#include "camera_controller.h"
#include "controller.h"

#include <fstream>
#include <sys/mman.h>
#include <thread>

using namespace libcamera;
using namespace std::chrono_literals;
using namespace std::chrono;

static CameraController* g_cameraController  = nullptr;

CameraController::CameraController() {
    this->m_camera = nullptr;
    this->m_imgFilename = "capture.bmp";
}

void CameraController::init() {
    g_cameraController = this;  // setting the global pointer because of the static callback
}

static void requestComplete(Request *request)
{
    g_cameraController->CameraRequestComplete(request);
}

void CameraController::CameraRequestComplete(Request* request) {
    std::cout << "Thread ID: " << std::this_thread::get_id() << "\n";
    std::cout << "requestComplete() 1" << std::endl;

    if (request->status() == Request::RequestCancelled)
        return;

    std::cout << "requestComplete() 2" << std::endl;
    const ControlList &metadata = request->metadata();

    bool aeConverged = false;
    bool afConverged = true; // Default to true if no AF

    auto aeStateOpt = metadata.get(controls::AeState);
    if (aeStateOpt) {
        int32_t aeState = *aeStateOpt;
        std::cout << "aeState = " << aeState << std::endl;
        if (aeState == 2) { // AeStateConverged
            aeConverged = true;
        }
    }

    auto afStateOpt = metadata.get(controls::AfState);
    if (afStateOpt) {
        int32_t afState = *afStateOpt;
        std::cout << "afState = " << afState << std::endl;
        if (afState == 2) { // AfStateFocused
            afConverged = true;
        } else {
            afConverged = false;
        }
    }

    steady_clock::time_point now = steady_clock::now();
    auto durationSec = duration_cast<seconds>(now - m_start_time).count();

    bool isLoopLongerThan5Sec = (durationSec >= 5);
    bool isSaveBmp = aeConverged || isLoopLongerThan5Sec;

    bool isTerminate;
    if (m_isTakeOnlySinglePhotoMode) {
        isTerminate = isSaveBmp; // terminate after saving single photo
        if (isTerminate) // reset the flag only if we are terminating
            m_isTakeOnlySinglePhotoMode = false;
    }
    else { // we should be in the MonitoringMode
        isTerminate = !m_isMonitoringMode;
    }

    //std::cout << "requestComplete() 3" << std::endl;
    // std::cout << "AE Converged: " << aeConverged << " isSaveBmp: " << isSaveBmp << " isTerminate: " << isTerminate << " m_isTakeOnlySinglePhotoMode: " << m_isTakeOnlySinglePhotoMode << std::endl;

    if (isSaveBmp) {
        // Converged or timeout, save the image
        const auto &buffers = request->buffers();
        auto bufferPair = *buffers.begin();
        FrameBuffer *buffer = bufferPair.second;
        const Stream *stream = bufferPair.first;

        const StreamConfiguration &cfg = stream->configuration();

        // Map the buffer memory
        const FrameBuffer::Plane &plane = buffer->planes()[0];
        void *data = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);
        if (data == MAP_FAILED) {
            std::cerr << "Failed to mmap buffer" << std::endl;
            return;
        }
        std::cout << "requestComplete() 5" << std::endl;

        // Save to BMP file
        std::ofstream out(m_imgFilename, std::ios::binary);
        // std::ofstream out("capture_2.bmp", std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open output file" << std::endl;
            munmap(data, plane.length);
            return;
        }

        uint32_t width = cfg.size.width;
        uint32_t height = cfg.size.height;
        uint32_t stride = cfg.stride;  // Use the stream's stride for row length
        uint32_t bpp = 3;  // Bytes per pixel for BGR888
        uint32_t row_size = ((width * bpp + 3) / 4) * 4;  // Padded to multiple of 4
        uint32_t image_size = row_size * height;
        uint32_t file_size = 54 + image_size;

        // BMP file header (14 bytes)
        out.write("BM", 2);
        out.write(reinterpret_cast<const char *>(&file_size), 4);
        uint32_t reserved = 0;
        out.write(reinterpret_cast<const char *>(&reserved), 4);
        uint32_t offset = 54;
        out.write(reinterpret_cast<const char *>(&offset), 4);

        // BMP info header (40 bytes)
        uint32_t info_size = 40;
        out.write(reinterpret_cast<const char *>(&info_size), 4);
        out.write(reinterpret_cast<const char *>(&width), 4);
        uint32_t bmp_height = height;  // Positive for bottom-up
        out.write(reinterpret_cast<const char *>(&bmp_height), 4);
        uint16_t planes = 1;
        out.write(reinterpret_cast<const char *>(&planes), 2);
        uint16_t bit_count = 24;
        out.write(reinterpret_cast<const char *>(&bit_count), 2);
        uint32_t compression = 0;
        out.write(reinterpret_cast<const char *>(&compression), 4);
        out.write(reinterpret_cast<const char *>(&image_size), 4);
        uint32_t x_ppm = 2835;  // 72 dpi
        out.write(reinterpret_cast<const char *>(&x_ppm), 4);
        uint32_t y_ppm = 2835;
        out.write(reinterpret_cast<const char *>(&y_ppm), 4);
        uint32_t clr_used = 0;
        out.write(reinterpret_cast<const char *>(&clr_used), 4);
        uint32_t clr_important = 0;
        out.write(reinterpret_cast<const char *>(&clr_important), 4);

        // Write pixel data (bottom-up)
        const char *img_data = static_cast<const char *>(data);
        for (int32_t y = height - 1; y >= 0; --y) {
            out.write(img_data + y * stride, width * bpp);
            // Add padding if necessary
            uint32_t pad = row_size - (width * bpp);
            if (pad > 0) {
                char pad_bytes[3] = {0, 0, 0};
                out.write(pad_bytes, pad);
            }
        }

        out.close();
        munmap(data, plane.length);
    }
    std::cout << "Image saved to capture.bmp" << std::endl;


    if (isTerminate) { // terminal condition for the infinte recursive loop
        std::cout << "Single photo taken, not continuing camera loop." << std::endl;
        // Signal completion
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done = true;
        }
        m_cv.notify_one();
        return; // terminate infinte recursive camera loop
    } else {
        if (m_isMonitoringMode) {
            m_controller->AnalyseImage(m_imgFilename);
        }

        std::cout << "Continuing camera loop..." << std::endl;
        // Reuse and queue again for camera loop
        request->reuse(Request::ReuseBuffers);
        m_camera->queueRequest(request);
        return;
    }
}

void CameraController::StartCameraLoop(Controller* controller) {
    std::cout << "CC: Starting camera loop...\n";
    m_controller = controller;
    m_isMonitoringMode = true;
    std::thread* thread1 = new std::thread(&CameraController::ActivateCamera, this); // Create one thread and start it immediately
    std::cout << "New thread created - main thread continues running...\n";
}

void CameraController::StopCameraLoop() {
    std::cout << "CC: Stopping camera loop...\n";
    m_isMonitoringMode = false;
}

void CameraController::TakePhotoToFile(std::string imgFilename) {

    std::cout << "Debug 1" << std::endl;

    this->m_imgFilename = imgFilename;
    m_isTakeOnlySinglePhotoMode = true; // take single photo;

    std::thread* thread1 = new std::thread(&CameraController::ActivateCamera, this); // Create one thread and start it immediately

    std::cout << "New thread created - main thread continues running...\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(9500));
}

void CameraController::ActivateCamera() {
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    int ret = cm->start();
    if (ret) {
        std::cerr << "Failed to start camera manager: " << ret << std::endl;
        return;
    }

    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cerr << "No cameras available" << std::endl;
        return;
    }

    std::string cameraId = cameras[0]->id();
    m_camera = cm->get(cameraId);

    ret = m_camera->acquire();
    if (ret) {
        std::cerr << "Failed to acquire camera: " << ret << std::endl;
        return;
    }

    std::cout << "Debug 2" << std::endl;

    std::unique_ptr<CameraConfiguration> config = m_camera->generateConfiguration({StreamRole::StillCapture});
    if (!config) {
        std::cerr << "Failed to generate configuration" << std::endl;
        return;
    }

    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.pixelFormat = formats::BGR888;
    streamConfig.size.width = 4608;
    streamConfig.size.height = 2592;

    CameraConfiguration::Status status = config->validate();
    std::cout << "Configuration validated: " << streamConfig.toString() << std::endl;

    ret = m_camera->configure(config.get());
    if (ret) {
        std::cerr << "Failed to configure camera: " << ret << std::endl;
        return;
    }

    std::cout << "Debug 3" << std::endl;

    std::unique_ptr<FrameBufferAllocator> allocator = std::make_unique<FrameBufferAllocator>(m_camera);
    std::cout << "Debug 4" << std::endl;
    ret = allocator->allocate(streamConfig.stream());
    std::cout << "Debug 5" << std::endl;
    if (ret < 0) {
        std::cerr << "Failed to allocate buffers: " << ret << std::endl;
        return;
    }
    std::cout << "Debug 6" << std::endl;

    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(streamConfig.stream());
    if (buffers.empty()) {
        std::cerr << "No buffers allocated" << std::endl;
        return;
    }
    std::cout << "Debug 7" << std::endl;

    std::unique_ptr<Request> request = m_camera->createRequest();
    if (!request) {
        std::cerr << "Failed to create request" << std::endl;
        return;
    }

    std::cout << "Debug 8" << std::endl;

    ret = request->addBuffer(streamConfig.stream(), buffers[0].get());
    if (ret) {
        std::cerr << "Failed to add buffer to request: " << ret << std::endl;
        return;
    }

    std::cout << "Debug 9" << std::endl;

    m_camera->requestCompleted.connect(requestComplete);

    m_start_time = steady_clock::now();

    ControlList controls;
    controls.set(controls::AeEnable, true);
    controls.set(controls::AwbEnable, true);
    controls.set(controls::AfMode, controls::AfModeAuto);

    std::cout << "Debug 10" << std::endl;

    ret = m_camera->start(&controls);
    if (ret) {
        std::cerr << "Failed to start camera: " << ret << std::endl;
        return;
    }

    ret = m_camera->queueRequest(request.get());
    if (ret) {
        std::cerr << "Failed to queue request: " << ret << std::endl;
        m_camera->stop();
        return;
    }

    std::cout << "Debug 11. Waiting for completion..." << std::endl;

    // Wait for completion
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [] { return g_cameraController->m_done; });
    }

    std::cout << "Debug 11...\n";

    m_camera->stop();
    m_camera->release();
    m_camera.reset();
    // cm->stop();  // should release camera manager, fix it later
    // cm.reset();
    // delete cm;
}

