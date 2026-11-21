#include "publisher.hpp"

InitParameters get_camera_init_params() {
    init_params_ = sl::InitParameters();
    init_params_.camera_resolution = sl::RESOLUTION::HD1200;
    init_params_.camera_fps = 60;
    init_params_.depth_mode = sl::DEPTH_MODE::NEURAL_LIGHT;  // lets us use PVA 
    init_params_.coordinate_units = sl::UNIT::METER;
    init_params_.coordinate_system = sl::COORDINATE_SYSTEM::IMAGE;
    init_params_.sdk_verbose = 1;
    init_params_.sdk_verbose_log_file = "zed_cam_sdk_log.txt";
    init_params_.depth_minimum_distance = 0.121f;
    init_params_.depth_maximum_distance = -1.0f;
    init_params_.camera_image_flip = sl::FLIP_MODE::OFF;
    init_params_.enable_right_side_measure = true;
    init_params_.depth_stabilization = 30;   // inc if lots of noise in depth map
    init_params_.input.setFromCameraID(0);
    init_params_.optional_settings_path = sl::String((rcpputils::fs::current_path() / "rover-config").string().c_str());
    init_params_.sensors_required = true;
    init_params_.enable_image_enhancement = true;
    init_params_.camera_disable_self_calib = false; // change to true when we have the calibration file for the camera done, for now use default
    // init_params_.optional_opencv_calibration_file = sl::String((std::filesystem::current_path() / "rover-config" / "zed_cam_calibration.xml").string().c_str());
    init_params_.open_timeout_sec = 0;
    init_params_.async_grab_camera_recovery = true;
    init_params_.grab_compute_capping_fps = init_params_.camera_fps;
    init_params_.enable_image_validity_check = true;
    init_params_.maximum_working_resolution = sl::Resolution(3840, 1200);
    init_params_.depth_precision = sl::DEPTH_PRECISION::FP16;
    init_params_.allow_depth_cuda_graph = false;   // depth gets faster but everything else on GPU will slow down
    init_params_.depth_compute_device = sl::DEPTH_COMPUTE_DEVICE::PVA;
    init_params_.imu_accelerometer_range = sl::ACCELEROMETER_RANGE::G_12;
    init_params_.imu_gyroscope_range = sl::GYROSCOPE_RANGE::DPS_2000;
    init_params_.imu_sampling_rate = sl::IMU_SAMPLING_RATE::HZ_400;
    return init_params_;
}