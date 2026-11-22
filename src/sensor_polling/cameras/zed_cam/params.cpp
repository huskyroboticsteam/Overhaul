#include "publisher.hpp"

sl::InitParameters get_camera_init_params() {
    sl::InitParameters init_params;
    init_params.camera_resolution = sl::RESOLUTION::HD1200;
    init_params.camera_fps = 60;
    init_params.depth_mode = sl::DEPTH_MODE::NEURAL;  
    init_params.coordinate_units = sl::UNIT::METER;
    init_params.coordinate_system = sl::COORDINATE_SYSTEM::IMAGE;
    init_params.sdk_verbose = 1;
    init_params.sdk_verbose_log_file = "zed_cam_sdk_log.txt";
    init_params.depth_minimum_distance = 0.121f;
    init_params.depth_maximum_distance = -1.0f;
    init_params.camera_image_flip = sl::FLIP_MODE::OFF;
    init_params.enable_right_side_measure = true;
    init_params.depth_stabilization = 30;   // inc if lots of noise in depth map
    init_params.input.setFromCameraID(0);
    init_params.optional_settings_path = sl::String((rcpputils::fs::current_path() / "rover-config").string().c_str());
    init_params.sensors_required = true;
    init_params.enable_image_enhancement = true;
    init_params.camera_disable_self_calib = false; // change to true when we have the calibration file for the camera done, for now use default
    // init_params.optional_opencv_calibration_file = sl::String((std::filesystem::current_path() / "rover-config" / "zed_cam_calibration.xml").string().c_str());
    init_params.open_timeout_sec = 0;
    init_params.async_grab_camera_recovery = true;
    init_params.grab_compute_capping_fps = init_params.camera_fps;
    init_params.enable_image_validity_check = true;
    init_params.maximum_working_resolution = sl::Resolution(3840, 1200);
    init_params.depth_precision = sl::DEPTH_PRECISION::FP16;
    init_params.allow_depth_cuda_graph = false;   // depth gets faster but everything else on GPU will slow down
    init_params.depth_compute_device = sl::DEPTH_COMPUTE_DEVICE::GPU;
    init_params.imu_accelerometer_range = sl::ACCELEROMETER_RANGE::G_12;
    init_params.imu_gyroscope_range = sl::GYROSCOPE_RANGE::DPS_2000;
    init_params.imu_sampling_rate = sl::IMU_SAMPLING_RATE::HZ_400;
    return init_params;
}

std::map<sl::VIDEO_SETTINGS, int> get_video_settings() {
    std::map<sl::VIDEO_SETTINGS, int> settings;
    settings.insert({sl::VIDEO_SETTINGS::AEC_AGC, 1});
    settings.insert({sl::VIDEO_SETTINGS::WHITEBALANCE_AUTO, 1});
    settings.insert({sl::VIDEO_SETTINGS::LED_STATUS, 1});
    settings.insert({sl::VIDEO_SETTINGS::DENOISING, 50});
    settings.insert({sl::VIDEO_SETTINGS::AE_ANTIBANDING, 1});
    return settings;
}

sl::RuntimeParameters get_runtime_params() {
    sl::RuntimeParameters runtime_params;
    runtime_params.enable_depth = true;
    runtime_params.enable_fill_mode = true;
    runtime_params.confidence_threshold = 95;
    runtime_params.texture_confidence_threshold = 95;
    runtime_params.remove_saturated_areas = false;
    runtime_params.measure3D_reference_frame = sl::REFERENCE_FRAME::CAMERA;
    return runtime_params;
}

sl::PositionalTrackingParameters get_positional_tracking_params() {
    sl::PositionalTrackingParameters tracking_params;
    tracking_params.initial_world_transform = sl::Transform(
        sl::Rotation::setEulerAngles(sl::Vector3<float>(0.0, 0.0, 0.0)), 
        sl::Translation(0, 0, 0)
    );
    tracking_params.enable_area_memory = true;
    tracking_params.enable_pose_smoothing = false;   // change if drift is bad
    tracking_params.set_floor_as_origin = false;    // floor is changing
    tracking_params.area_file_path = sl::String();
    tracking_params.enable_imu_fusion = true;
    tracking_params.depth_min_range = -1;
    tracking_params.set_gravity_as_origin = true;
    tracking_params.mode = sl::POSITIONAL_TRACKING_MODE::GEN_3;
    tracking_params.enable_localization_only = false;
    tracking_params.enable_2d_ground_mode = false;
    tracking_params.compute_preference = sl::COMPUTE_PREFERENCE::PREFER_ACCELERATORS; // if latency is too high or cpu is free do PREFER_CPU    
    return tracking_params;
}

sl::ObjectDetectionParameters get_object_detection_params() {
    sl::ObjectDetectionParameters obj_det_params;
    obj_det_params.enable_tracking = true;
    obj_det_params.enable_segmentation = false;
    obj_det_params.max_range = 20.0f;
    sl::BatchParameters batch_params;
    batch_params.enable = false; 
    obj_det_params.batch_parameters = batch_params;
    obj_det_params.filtering_mode = sl::OBJECT_FILTERING_MODE::NMS3D;
    obj_det_params.prediction_timeout_s = 0.5f;
    obj_det_params.detection_model = sl::OBJECT_DETECTION_MODEL::CUSTOM_YOLOLIKE_BOX_OBJECTS;
    obj_det_params.allow_reduced_precision_inference = false;
    obj_det_params.instance_module_id = 0;
    obj_det_params.custom_model_path = sl::String((rcpputils::fs::current_path() / "rover-config" / "zed_cam_custom_model.onnx").string().c_str());
    obj_det_params.custom_onnx_dynamic_input_shape = sl::Resolution
    return obj_det_params;
}

// Obj det runtime params
// body tracking
// sensors