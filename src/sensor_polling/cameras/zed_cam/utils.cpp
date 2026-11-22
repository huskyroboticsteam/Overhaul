#include "publisher.hpp"

std::string interpret_error_code(sl::ERROR_CODE err) {
    switch (err){
        case sl::ERROR_CODE::POTENTIAL_CALIBRATION_ISSUE:
            return "The camera has a potential calibration issue.";
        case sl::ERROR_CODE::CONFIGURATION_FALLBACK:
            return "The operation could not proceed with the target configuration but did succeed with a fallback.";
        case sl::ERROR_CODE::SENSORS_DATA_REQUIRED:
            return "The input data does not contain high-frequency sensor data. This usually means newer SVO or streaming data is required. This module needs inertial data in its input.";
        case sl::ERROR_CODE::CORRUPTED_FRAME:
            return "The image could be corrupted.";
        case sl::ERROR_CODE::CAMERA_REBOOTING:
            return "The camera is currently rebooting.";
        case sl::ERROR_CODE::SUCCESS:
            return "Standard code for successful behavior.";
        case sl::ERROR_CODE::FAILURE:
            return "Standard code for unsuccessful behavior.";
        case sl::ERROR_CODE::NO_GPU_COMPATIBLE:
            return "No GPU was found, or the device's CUDA capability is not supported.";
        case sl::ERROR_CODE::NOT_ENOUGH_GPU_MEMORY:
            return "There is not enough GPU memory for this depth mode. Try a different mode, such as PERFORMANCE, or increase the minimum depth value.";
        case sl::ERROR_CODE::CAMERA_NOT_DETECTED:
            return "No camera was detected.";
        case sl::ERROR_CODE::SENSORS_NOT_INITIALIZED:
            return "The MCU that controls the sensors module has an invalid serial number. You can try to recover it by launching the ZED Diagnostic tool from the command line with the -r option.";
        case sl::ERROR_CODE::SENSORS_NOT_AVAILABLE:
            return "A camera with sensors was detected, but its sensors (IMU, barometer, etc.) cannot be opened. Unplug and reconnect the camera.";
        case sl::ERROR_CODE::INVALID_RESOLUTION:
            return "The resolution parameter is invalid, such as when upscaling beyond the original image size in Camera::retrieveImage.";
        case sl::ERROR_CODE::LOW_USB_BANDWIDTH:
            return "There is insufficient bandwidth for the camera to work correctly. This can happen when using multiple cameras or a USB 2.0 port.";
        case sl::ERROR_CODE::CALIBRATION_FILE_NOT_AVAILABLE:
            return "The camera's calibration file was not found on the host machine. Use ZED Explorer or ZED Calibration to download the factory calibration file.";
        case sl::ERROR_CODE::INVALID_CALIBRATION_FILE:
            return "The calibration file is invalid. Try downloading the factory calibration file or recalibrating your camera with ZED Calibration.";
        case sl::ERROR_CODE::INVALID_SVO_FILE:
            return "The provided SVO file is invalid.";
        case sl::ERROR_CODE::SVO_RECORDING_ERROR:
            return "An error occurred while recording an SVO, such as insufficient free storage or an invalid file.";
        case sl::ERROR_CODE::SVO_UNSUPPORTED_COMPRESSION:
            return "An SVO-related error occurred because NVIDIA-based compression could not be loaded.";
        case sl::ERROR_CODE::END_OF_SVOFILE_REACHED:
            return "The end of the SVO file has been reached. No frame will be available until the SVO position is reset.";
        case sl::ERROR_CODE::INVALID_COORDINATE_SYSTEM:
            return "The requested coordinate system is not available.";
        case sl::ERROR_CODE::INVALID_FIRMWARE:
            return "The camera firmware is out of date. Update it to the latest version.";
        case sl::ERROR_CODE::INVALID_FUNCTION_PARAMETERS:
            return "Invalid parameters were given for the function.";
        case sl::ERROR_CODE::CUDA_ERROR:
            return "A CUDA error was detected in Camera::grab() or Camera::retrieveXXX(). Enable verbose mode in Camera::open() for more information.";
        case sl::ERROR_CODE::CAMERA_NOT_INITIALIZED:
            return "The ZED SDK is not initialized. Camera::open() may be missing.";
        case sl::ERROR_CODE::NVIDIA_DRIVER_OUT_OF_DATE:
            return "The NVIDIA driver is too old and is not compatible with the current CUDA version.";
        case sl::ERROR_CODE::INVALID_FUNCTION_CALL:
            return "The function call is not valid in the current context. Camera::open() may be missing.";
        case sl::ERROR_CODE::CORRUPTED_SDK_INSTALLATION:
            return "The ZED SDK could not load its dependencies, or some assets are missing. Reinstall the SDK or check for missing dependencies such as cuDNN or TensorRT.";
        case sl::ERROR_CODE::INCOMPATIBLE_SDK_VERSION:
            return "The installed ZED SDK is incompatible with the version used to compile the program.";
        case sl::ERROR_CODE::INVALID_AREA_FILE:
            return "The given area file does not exist. Check the path.";
        case sl::ERROR_CODE::INCOMPATIBLE_AREA_FILE:
            return "The area file does not contain enough data, or the depth mode used to create it differs from the one currently set.";
        case sl::ERROR_CODE::CAMERA_FAILED_TO_SETUP:
            return "Failed to open the camera at the proper resolution. Try another resolution or check that the UVC driver is properly installed.";
        case sl::ERROR_CODE::CAMERA_DETECTION_ISSUE:
            return "The camera cannot be opened. Try reconnecting it to another port or flipping the USB-C connector, if applicable.";
        case sl::ERROR_CODE::CANNOT_START_CAMERA_STREAM:
            return "The camera stream cannot be started. Make sure the camera is not being used by another process or blocked by a firewall or antivirus.";
        case sl::ERROR_CODE::NO_GPU_DETECTED:
            return "No GPU was found. CUDA cannot list it. This may be a driver or reboot issue.";
        case sl::ERROR_CODE::PLANE_NOT_FOUND:
            return "No plane was found at the specified scene location, or the detected floor plane does not match the one previously given.";
        case sl::ERROR_CODE::MODULE_NOT_COMPATIBLE_WITH_CAMERA:
            return "The module is not compatible with the camera model. The MODEL::ZED has no IMU and does not support AI modules.";
        case sl::ERROR_CODE::MOTION_SENSORS_REQUIRED:
            return "The module requires sensors to be enabled.";
        case sl::ERROR_CODE::MODULE_NOT_COMPATIBLE_WITH_CUDA_VERSION:
            return "The module requires a newer version of CUDA.";
        case sl::ERROR_CODE::DRIVER_FAILURE:
            return "Driver initialization failed. For GMSL cameras, try restarting zed_x_daemon.service.";
        default:
            return "Unknown error code.";
    }
}