#ifndef LIDARPROTOCOL_H
#define LIDARPROTOCOL_H
#include <QtGlobal>

const quint8 SYNC = 0xa5;
const quint8 START_FLAG = 0x5a;

// Commands without payload and response
const quint8 RPLIDAR_CMD_STOP = 0x25;
const quint8 RPLIDAR_CMD_SCAN = 0x20;
const quint8 RPLIDAR_CMD_FORCE_SCAN = 0x21;
const quint8 RPLIDAR_CMD_RESET = 0x40;

// Commands without payload but have response
const quint8 RPLIDAR_CMD_GET_DEVICE_INFO = 0x50;
const quint8 RPLIDAR_CMD_GET_DEVICE_HEALTH = 0x52;
const quint8 RPLIDAR_CMD_GET_SAMPLERATE = 0x59; //added in fw 1.17;
const quint8 RPLIDAR_CMD_HQ_MOTOR_SPEED_CTRL = 0xA8;

// Commands with payload and have response
const quint8 RPLIDAR_CMD_EXPRESS_SCAN = 0x82; //added in fw 1.17;
const quint8 RPLIDAR_CMD_HQ_SCAN = 0x83; //added in fw 1.24;
const quint8 RPLIDAR_CMD_GET_LIDAR_CONF = 0x84; //added in fw 1.24;
const quint8 RPLIDAR_CMD_SET_LIDAR_CONF = 0x85; //added in fw 1.24;

//add for A2 to set RPLIDAR motor pwm when using accessory board
const quint8 RPLIDAR_CMD_SET_MOTOR_PWM = 0xF0;
const quint8 RPLIDAR_CMD_GET_ACC_BOARD_FLAG = 0xFF;


// Payloads
// ------------------------------------------
const quint8 RPLIDAR_EXPRESS_SCAN_MODE_NORMAL = 0 ;
const quint8 RPLIDAR_EXPRESS_SCAN_MODE_FIXANGLE = 0;  // won't been supported but keep to prevent build fail;
                                                            //for express working flag(extending express scan protocol)

const quint8 RPLIDAR_EXPRESS_SCAN_FLAG_BOOST = 0x0001 ;
const quint8 RPLIDAR_EXPRESS_SCAN_FLAG_SUNLIGHT_REJECTION = 0x0002;

//for ultra express working flag
const quint8 RPLIDAR_ULTRAEXPRESS_SCAN_FLAG_STD = 0x0001 ;
const quint8 RPLIDAR_ULTRAEXPRESS_SCAN_FLAG_HIGH_SENSITIVITY = 0x0002;
const quint8 RPLIDAR_HQ_SCAN_FLAG_CCW = (0x1<<0);
const quint8 RPLIDAR_HQ_SCAN_FLAG_RAW_ENCODER = (0x1<<1);
const quint8 RPLIDAR_HQ_SCAN_FLAG_RAW_DISTANCE = (0x1<<2);
const int MAX_MOTOR_PWM = 1023;
const int DEFAULT_MOTOR_PWM = 660;

// Response
// ------------------------------------------
const quint8 RPLIDAR_ANS_TYPE_DEVINFO = 0x4;
const quint8 RPLIDAR_ANS_TYPE_DEVHEALTH = 0x6;
const quint8 RPLIDAR_ANS_TYPE_MEASUREMENT = 0x81;

// Added in FW ver 1.17
const quint8 RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED = 0x82;
const quint8 RPLIDAR_ANS_TYPE_MEASUREMENT_HQ = 0x83;


// Added in FW ver 1.17
const quint8 RPLIDAR_ANS_TYPE_SAMPLE_RATE = 0x15;

//added in FW ver 1.23alpha
const quint8 RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA = 0x84;

//added in FW ver 1.24
const quint8 RPLIDAR_ANS_TYPE_GET_LIDAR_CONF = 0x20;
const quint8 RPLIDAR_ANS_TYPE_SET_LIDAR_CONF = 0x21;
const quint8 RPLIDAR_ANS_TYPE_ACC_BOARD_FLAG = 0xFF;
const quint8 RPLIDAR_RESP_ACC_BOARD_FLAG_MOTOR_CTRL_SUPPORT_MASK = (0x1);

const quint8 RPLIDAR_STATUS_OK = 0x0;
const quint8 RPLIDAR_STATUS_WARNING = 0x1;
const quint8 RPLIDAR_STATUS_ERROR = 0x2;
const quint8 RPLIDAR_RESP_MEASUREMENT_SYNCBIT = (0x1<<0);
const quint8 RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT = 2;
const quint8 RPLIDAR_RESP_HQ_FLAG_SYNCBIT = (0x1<<0);
const quint8 RPLIDAR_RESP_MEASUREMENT_CHECKBIT = (0x1<<0);
const quint8 RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT = 1;

//[distance_sync flags]
const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_ANGLE_MASK = (0x3);
const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_DISTANCE_MASK = (0xFC);

const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_1 = 0xA;
const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_2 = 0x5;
const quint8 RPLIDAR_RESP_MEASUREMENT_HQ_SYNC = 0xA5;
const quint32 RPLIDAR_RESP_MEASUREMENT_EXP_SYNCBIT = (0x1<<15);

const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_ULTRA_MAJOR_BITS = 12;
const quint8 RPLIDAR_RESP_MEASUREMENT_EXP_ULTRA_PREDICT_BITS = 10;

const quint32 RPLIDAR_CONF_ANGLE_RANGE = 0x00000000;
const quint32 RPLIDAR_CONF_DESIRED_ROT_FREQ = 0x00000001;
const quint32 RPLIDAR_CONF_SCAN_COMMAND_BITMAP = 0x00000002;
const quint32 RPLIDAR_CONF_MIN_ROT_FREQ = 0x00000004;
const quint32 RPLIDAR_CONF_MAX_ROT_FREQ = 0x00000005;
const quint32 RPLIDAR_CONF_MAX_DISTANCE = 0x00000060;
const quint32 RPLIDAR_CONF_SCAN_MODE_COUNT = 0x00000070;
const quint32 RPLIDAR_CONF_SCAN_MODE_US_PER_SAMPLE = 0x00000071;
const quint32 RPLIDAR_CONF_SCAN_MODE_MAX_DISTANCE = 0x00000074;
const quint32 RPLIDAR_CONF_SCAN_MODE_ANS_TYPE = 0x00000075;
const quint32 RPLIDAR_CONF_SCAN_MODE_TYPICAL = 0x0000007C;
const quint32 RPLIDAR_CONF_SCAN_MODE_NAME = 0x0000007F;
const quint32 RPLIDAR_EXPRESS_SCAN_STABILITY_BITMAP = 4;
const quint32 RPLIDAR_EXPRESS_SCAN_SENSITIVITY_BITMAP = 5;


// Definition of the variable bit scale encoding mechanism
const quint8 RPLIDAR_VARBITSCALE_X2_SRC_BIT = 9;
const quint8 RPLIDAR_VARBITSCALE_X4_SRC_BIT = 11;
const quint8 RPLIDAR_VARBITSCALE_X8_SRC_BIT = 12;
const quint8 RPLIDAR_VARBITSCALE_X16_SRC_BIT = 14;
const quint32 RPLIDAR_VARBITSCALE_X2_DEST_VAL = 512;
const quint32 RPLIDAR_VARBITSCALE_X4_DEST_VAL = 1280;
const quint32 RPLIDAR_VARBITSCALE_X8_DEST_VAL = 1792;
const quint32 RPLIDAR_VARBITSCALE_X16_DEST_VAL = 3328;

#endif // LIDARPROTOCOL_H
