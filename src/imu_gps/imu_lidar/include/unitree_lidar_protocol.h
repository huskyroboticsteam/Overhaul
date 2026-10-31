#ifndef UNILIDAR_SDK2_PROTOCOL_HPP_
#define UNILIDAR_SDK2_PROTOCOL_HPP_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef __EXTERN_C__
namespace unilidar_sdk2{
#endif

#define FRAME_HEADER_ARRAY_0 0x55
#define FRAME_HEADER_ARRAY_1 0xAA
#define FRAME_HEADER_ARRAY_2 0x05
#define FRAME_HEADER_ARRAY_3 0x0A

#define FRAME_TAIL_ARRAY_0 0x00
#define FRAME_TAIL_ARRAY_1 0xFF

#define LIDAR_USER_CMD_PACKET_TYPE 100
#define LIDAR_ACK_DATA_PACKET_TYPE 101
#define LIDAR_POINT_DATA_PACKET_TYPE 102
#define LIDAR_2D_POINT_DATA_PACKET_TYPE 103
#define LIDAR_IMU_DATA_PACKET_TYPE 104
#define LIDAR_VERSION_PACKET_TYPE 105
#define LIDAR_TIME_STAMP_PACKET_TYPE 106
#define LIDAR_WORK_MODE_CONFIG_PACKET_TYPE 107
#define LIDAR_IP_ADDRESS_CONFIG_PACKET_TYPE 108
#define LIDAR_MAC_ADDRESS_CONFIG_PACKET_TYPE 109

#define LIDAR_COMMAND_PACKET_TYPE 2000
#define LIDAR_PARAM_DATA_PACKET_TYPE 2001

#define CMD_RESET_TYPE 1
#define CMD_PARAM_SAVE 2
#define CMD_PARAM_GET 3
#define CMD_VERSION_GET 4
#define CMD_STANDBY_TYPE 5
#define CMD_LATENCY_TYPE 6
#define CMD_CONFIG_RESET 7

#define USER_CMD_RESET_TYPE 1
#define USER_CMD_STANDBY_TYPE 2
#define USER_CMD_VERSION_GET 3
#define USER_CMD_LATENCY_TYPE 4
#define USER_CMD_CONFIG_RESET 5
#define USER_CMD_CONFIG_GET 6
#define USER_CMD_CONFIG_AUTO_STANDBY 7

#define ACK_SUCCESS 1
#define ACK_CRC_ERROR 2
#define ACK_HEADER_ERROR 3
#define ACK_BLOCK_ERROR 4
#define ACK_WAIT_ERROR 5

typedef struct
{
    uint8_t header[4];
    uint32_t packet_type;
    uint32_t packet_size;
}FrameHeader;

typedef struct
{
    uint32_t crc32;
    uint32_t msg_type_check;
    uint8_t reserve[2];
    uint8_t tail[2];
}FrameTail;

typedef struct
{
    uint32_t sec;
    uint32_t nsec;
}TimeStamp;

typedef struct
{
    uint32_t seq;
    uint32_t payload_size;
    TimeStamp stamp;
}DataInfo;

typedef struct
{
    float a_axis_dist;
    float b_axis_dist;
    float theta_angle_bias;
    float alpha_angle_bias;
    float beta_angle;
    float xi_angle;
    float range_bias;
    float range_scale;
}LidarCalibParam;

typedef struct
{
    uint32_t sys_rotation_period;
    uint32_t com_rotation_period;
    float dirty_index;
    float packet_lost_up;
    float packet_lost_down;
    float apd_temperature;
    float apd_voltage;
    float laser_voltage;
    float imu_temperature;
}LidarInsideState;

typedef struct
{
    DataInfo info;
    LidarInsideState state;
    LidarCalibParam param;
    float com_horizontal_angle_start;
    float com_horizontal_angle_step;
    float scan_period;
    float range_min;
    float range_max;
    float angle_min;
    float angle_increment;
    float time_increment;
    uint32_t point_num;
    uint16_t ranges[300];
    uint8_t intensities[300];
}LidarPointData;

typedef struct
{
    FrameHeader header;
    LidarPointData data;
    FrameTail tail;
}LidarPointDataPacket;

typedef struct
{
    DataInfo info;
    LidarInsideState state;
    LidarCalibParam param;
    float scan_period;
    float range_min;
    float range_max;
    float angle_min;
    float angle_increment;
    float time_increment;
    uint32_t point_num;
    uint16_t ranges[1800];
    uint8_t intensities[1800];
}Lidar2DPointData;

typedef struct
{
    FrameHeader header;
    Lidar2DPointData data;
    FrameTail tail;
}Lidar2DPointDataPacket;

typedef struct
{
    FrameHeader header;
    TimeStamp data;
    FrameTail tail;
}LidarTimeStampPacket;

typedef struct
{
    DataInfo info;
    float quaternion[4];
    float angular_velocity[3];
    float linear_acceleration[3];
}LidarImuData;

typedef struct
{
    FrameHeader header;
    LidarImuData data;
    FrameTail tail;
}LidarImuDataPacket;

typedef struct
{
    uint32_t packet_type;
    uint32_t cmd_type;
    uint32_t cmd_value;
    uint32_t status;
}LidarAckData;

typedef struct
{
    FrameHeader header;
    LidarAckData data;
    FrameTail tail;
}LidarAckDataPacket;

typedef struct
{
    uint8_t hw_version[4];
    uint8_t sw_version[4];
    uint8_t name[24];
    uint8_t date[8];
    uint8_t reserve[40];
}LidarVersionData;

typedef struct
{
    FrameHeader header;
    LidarVersionData data;
    FrameTail tail;
}LidarVersionDataPacket;

typedef struct
{
    uint8_t lidar_ip[4];
    uint8_t user_ip[4];
    uint8_t gateway[4];
    uint8_t subnet_mask[4];
    uint16_t lidar_port;
    uint16_t user_port;
}LidarIpAddressConfig;

typedef struct
{
    FrameHeader header;
    LidarIpAddressConfig data;
    FrameTail tail;
}LidarIpAddressConfigPacket;

typedef struct
{
    uint8_t mac[6];
    uint8_t reserve[2];
}LidarMacAddressConfig;

typedef struct
{
    FrameHeader header;
    LidarMacAddressConfig data;
    FrameTail tail;
}LidarMacAddressConfigPacket;

typedef struct
{
    uint32_t mode;
}LidarWorkModeConfig;

typedef struct
{
    FrameHeader header;
    LidarWorkModeConfig data;
    FrameTail tail;
}LidarWorkModeConfigPacket;

typedef struct
{
    uint32_t cmd_type;
    uint32_t cmd_value;
}LidarUserCtrlCmd;

typedef struct
{
    FrameHeader header;
    LidarUserCtrlCmd data;
    FrameTail tail;
}LidarUserCtrlCmdPacket;

#ifndef __EXTERN_C__
}
#endif

#endif