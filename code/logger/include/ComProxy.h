
#include <Arduino.h>
#include <SoftwareSerial.h>

#define EVT_MSG_MOTOR_INIT_OK				1
#define EVT_MSG_CONFIG_READ					2
#define EVT_MSG_CONFIG_RESET				3
#define EVT_MSG_CONFIG_WRITTEN				4

#define EVT_ERROR_INIT_MOTOR				64
#define EVT_ERROR_CHANGE_TARGET_SPEED		65
#define EVT_ERROR_CHANGE_TARGET_CURRENT		66
#define EVT_ERROR_READ_MOTOR_STATUS			67
#define EVT_ERROR_READ_MOTOR_CURRENT		68
#define EVT_ERROR_READ_MOTOR_VOLTAGE		69

#define EVT_ERROR_CONFIG_READ_EEPROM		70
#define EVT_ERROR_CONFIG_WRITE_EEPROM		71
#define EVT_ERROR_CONFIG_ERASE_EEPROM		72
#define EVT_ERROR_CONFIG_VERSION			73
#define EVT_ERROR_CONFIG_CHECKSUM			74
#define EVT_ERROR_THROTTLE_LOW_LIMIT		75
#define EVT_ERROR_THROTTLE_HIGH_LIMIT		76


#define EVT_DATA_TARGET_CURRENT				128
#define EVT_DATA_TARGET_SPEED				129
#define EVT_DATA_MOTOR_STATUS				130
#define EVT_DATA_ASSIST_LEVEL				131
#define EVT_DATA_OPERATION_MODE				132
#define EVT_DATA_WHEEL_SPEED_PPM			133
#define EVT_DATA_LIGHTS						134
#define EVT_DATA_TEMPERATURE				135
#define EVT_DATA_THERMAL_LIMITING			136
#define EVT_DATA_SPEED_LIMITING				137
#define EVT_DATA_MAX_CURRENT_ADC_REQUEST	138
#define EVT_DATA_MAX_CURRENT_ADC_RESPONSE	139
#define EVT_DATA_MAIN_LOOP_TIME				140
#define EVT_DATA_THROTTLE_ADC				141 
#define EVT_DATA_LVC_LIMITING				142
#define EVT_DATA_SHIFT_SENSOR				143


class ComProxy
{
public:
    struct Event
    {
        uint32_t timestamp;
        uint8_t id;
        int16_t data;
    };

    static void printFormat(Stream& stream, const Event& e);

    ComProxy(Stream& controller, Stream& display, Stream& log);

    bool connect();
    bool isConnected() const;

    void process();

    bool hasLogEvent() const;
    bool getLogEvent(Event& e);

private:
    void processControllerTx();
    void processDisplayTx();

    void flushInterceptbuffer();
    bool interceptMessage();

    int tryProcessControllerMessage();
    int processReadRequestResponse();
    int processWriteRequestResponse();
    int processEventLogMessage();

private:
    Stream& _log;
    Stream& _controller;
    Stream& _display;
    bool _connected;
    uint8_t _msgLen;
    uint8_t _msgBuf[128];
    uint32_t _lastRecv;

    bool _hasEvent;
    Event _event;
};
