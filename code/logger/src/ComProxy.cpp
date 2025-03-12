#include "ComProxy.h"

#define KEEP		            0
#define FAIL		            -1


#define REQUEST_TYPE_READ						0x01
#define REQUEST_TYPE_WRITE						0x02

// Firmware config tool communication (only expected opcodes)
#define OPCODE_READ_FW_VERSION					0x01
#define OPCODE_READ_EVTLOG_ENABLE				0x02

#define OPCODE_WRITE_EVTLOG_ENABLE				0xf0

#define EVENT_LOG_ENTRY         				0xee
#define EVENT_LOG_DATA_ENTRY 					0xed


static uint8_t computeChecksum(uint8_t* buf, uint8_t length)
{
	uint8_t result = 0;
	for (uint8_t i = 0; i < length; ++i)
	{
		result += buf[i];
	}

	return result;
}

int verifyControllerMessage(uint8_t* buf, uint8_t length, uint8_t requiredLength, Stream& log)
{
	if (length < requiredLength)
	{
		return KEEP;
	}

	uint8_t checksum = computeChecksum(buf, requiredLength - 1);
	if (checksum == buf[requiredLength - 1])
	{
		return requiredLength;
	}
	/*else
	{
		log.print("Checksum mismatch, computed=");
		log.print(checksum, HEX);
		log.print(" message=");
		for (uint8_t i = 0; i < requiredLength; i++)
		{
			log.print(buf[i], HEX);
			log.print(" ");
		}
		log.println();
	}*/

	return FAIL;
}


void ComProxy::printFormat(Stream& stream, const Event& evt)
{
	switch (evt.id)
	{
	case EVT_MSG_MOTOR_INIT_OK:
		stream.print(F("Motor initialization successful."));
		break;
	case EVT_MSG_CONFIG_READ:
		stream.print(F("Successfully read configuration from eeprom."));
		break;
	case EVT_MSG_CONFIG_RESET:
		stream.print(F("Configuration reset performed."));
		break;
	case EVT_MSG_CONFIG_WRITTEN:
		stream.print(F("Configuration written to eeprom."));
		break;
	case EVT_ERROR_INIT_MOTOR:
		stream.print(F("Failed to perform motor controller initialization."));
		break;
	case EVT_ERROR_CHANGE_TARGET_CURRENT:
		stream.print(F("Failed to set motor target current on motor controller."));
		break;
	case EVT_ERROR_CHANGE_TARGET_SPEED:
		stream.print(F("Failed to set motor target speed on motor controller."));
		break;
	case EVT_ERROR_READ_MOTOR_STATUS:
		stream.print(F("Failed to read status from motor controller."));
		break;
	case EVT_ERROR_READ_MOTOR_CURRENT:
		stream.print(F("Failed to read current from motor controller."));
		break;
	case EVT_ERROR_READ_MOTOR_VOLTAGE:
		stream.print(F("Failed to read voltage from motor controller."));
		break;
	case EVT_ERROR_CONFIG_READ_EEPROM:
		stream.print(F("Failed to read config from eeprom."));
		break;
	case EVT_ERROR_CONFIG_WRITE_EEPROM:
		stream.print(F("Failed to write config to eeprom."));
		break;
	case EVT_ERROR_CONFIG_ERASE_EEPROM:
		stream.print(F("Failed to erase eeprom before writing config."));
		break;
	case EVT_ERROR_CONFIG_VERSION:
		stream.print(F("Configuration read from eeprom is of the wrong version."));
		break;
	case EVT_ERROR_CONFIG_CHECKSUM:
		stream.print(F("Failed to verify checksum on configuration read from eeprom."));
		break;
	case EVT_ERROR_THROTTLE_LOW_LIMIT:
		stream.print(F("Invalid throttle reading, below low limit, check throttle."));
		break;
	case EVT_ERROR_THROTTLE_HIGH_LIMIT:
		stream.print(F("Invalid throttle reading, above high limit, check throttle."));
		break;

	case EVT_DATA_TARGET_CURRENT:
		stream.print(F("Motor target current changed to "));
		stream.print(evt.data);
		stream.print(F("%"));
		break;
	case EVT_DATA_TARGET_SPEED:
		stream.print(F("Motor target speed changed to "));
		stream.print((evt.data * 100) / 255);
		stream.print(F("%."));
		break;
	case EVT_DATA_MOTOR_STATUS:
		stream.print(F("Motor controller status changed to "));
		stream.print(evt.data, HEX);
		stream.print(F("."));
		break;
	case EVT_DATA_ASSIST_LEVEL:
		stream.print(F("Assist level changed to "));
		stream.print(evt.data);
		stream.print(F("."));
		break;
	case EVT_DATA_OPERATION_MODE:
		stream.print(F("Operation mode changed to "));
		stream.print(evt.data);
		stream.print(F("."));
		break;
	case EVT_DATA_WHEEL_SPEED_PPM:
		stream.print(F("Max wheel speed changed to "));
		stream.print(evt.data);
		stream.print(F(" rpm."));
		break;
	case EVT_DATA_LIGHTS:
		stream.print(F("Lights status changed to "));
		stream.print(evt.data);
		stream.print(F("."));
		break;
	case EVT_DATA_TEMPERATURE:
		stream.print(F("Motor controller temperature changed to "));
		stream.print(evt.data);
		stream.print(F("C."));
		break;
	case EVT_DATA_THERMAL_LIMITING:
		if (evt.data != 0)
		{
			stream.print(F("Thermal limit reached, power reduced to 50%."));
		}
		else
		{
			stream.print(F("Thermal limiting removed."));
		}
		break;
	case EVT_DATA_SPEED_LIMITING:
		if (evt.data != 0)
		{
			stream.print(F("Speed limiting activated."));
		}
		else
		{
			stream.print(F("Speed limiting deactivated."));
		}
		break;
	case EVT_DATA_MAX_CURRENT_ADC_REQUEST:
		stream.print(F("Requesting to configure max current on motor controller mcu, adc="));
		stream.print(evt.data);
		stream.print(".");
		break;
	case EVT_DATA_MAX_CURRENT_ADC_RESPONSE:
		stream.print(F("Max current configured on motor controller mcu, response was adc="));
		stream.print(evt.data);
		stream.print(F("."));
		break;
	case EVT_DATA_MAIN_LOOP_TIME:
		stream.print(F("Main loop, interval="));
		stream.print(evt.data);
		stream.print(F("ms."));
		break;
	case EVT_DATA_THROTTLE_ADC:
		stream.print(F("Throttle adc, value="));
		stream.print(evt.data);
		stream.print(F("."));
		break;
    case EVT_DATA_LVC_LIMITING:
        if (evt.data != 0)
        {
            stream.print(F("Low voltage limiting activated, voltage="));
            stream.print(evt.data / 10.f);
            stream.print(F("."));
        }
        else
        {
            stream.print("Low voltage limiting deactivated.");
        }
        break;
    case EVT_DATA_SHIFT_SENSOR:
        if (evt.data.Value != 0)
        {
            stream.print("Shift sensor power ramp started.");
        }
        else
        {
            stream.print("Shift sensor power ramp ended.");
        }
        break;
	default:
		stream.print(F("Unknown entry, id="));
		stream.print(evt.id);
		stream.print(F(" data="));
		stream.print(evt.data);
		break;
	}
}



ComProxy::ComProxy(Stream& controller, Stream& display, Stream& log)
    : _log(log)
	, _controller(controller)
	, _display(display)
	, _connected(false)
    , _msgLen(0)
    , _lastRecv(0)
	, _hasEvent(false)
{ }

bool ComProxy::isConnected() const
{
	return _connected;
}

bool ComProxy::connect()
{
	uint8_t buffer[4];

	buffer[0] = REQUEST_TYPE_WRITE;
	buffer[1] = OPCODE_WRITE_EVTLOG_ENABLE;
	buffer[2] = 1;
	buffer[3] = computeChecksum(buffer, 3);

	_controller.write(buffer, 4);

	uint32_t now = millis();
	while(!_connected && (millis() - now) < 1000)
	{
		processControllerTx();
	}

	return _connected;
}

void ComProxy::process()
{  
	processControllerTx();
	processDisplayTx();
}
   
bool ComProxy::hasLogEvent() const
{
	return _hasEvent;
}

bool ComProxy::getLogEvent(Event& e)
{
	if (_hasEvent)
	{
		e = _event;
		_hasEvent = false;
		return true;
	}
	
	return false;
}


void ComProxy::processControllerTx()
{
	int b = -1;
	while ((b = _controller.read()) != -1)
	{
		_lastRecv = millis();

		_msgBuf[_msgLen++] = b;

		int res;
		while (_msgLen > 0 && (res = tryProcessControllerMessage()) != KEEP)
		{
			if (res == FAIL)
			{
				_display.write(_msgBuf[0]);
				if (_msgLen > 1)
				{
					memcpy(_msgBuf, _msgBuf + 1, _msgLen - 1);
				}				
				_msgLen--;
				continue;
			}
			else if (res >= 0)
			{
				// succesfully intercepted
				_msgLen = 0;
				break;
			}		
		}
	}

	if (_msgLen > 0 && millis() - _lastRecv > 20)
	{
		flushInterceptbuffer();
	}
}

void ComProxy::processDisplayTx()
{
	int b = -1;
	while ((b = _display.read()) != -1)
	{
		_controller.write((uint8_t)b);
	}
}


void ComProxy::flushInterceptbuffer()
{
	for (uint8_t i = 0; i < _msgLen; i++)
	{
		_display.write(_msgBuf[i]);
	}

	_msgLen = 0;
}


int ComProxy::tryProcessControllerMessage()
 {
    if (_msgLen < 1)
	{
		return KEEP;
	}

	switch (_msgBuf[0])
	{
	case REQUEST_TYPE_READ:
		return processReadRequestResponse();
	case REQUEST_TYPE_WRITE:
		return processWriteRequestResponse();
    case EVENT_LOG_ENTRY:
    case EVENT_LOG_DATA_ENTRY:
        return processEventLogMessage();
	}

	return FAIL; // unknown message, forward to display
}

int ComProxy::processReadRequestResponse()
{
	if (_msgLen < 2)
	{
		return KEEP;
	}

	switch (_msgBuf[1])
	{
	case OPCODE_READ_FW_VERSION:
		return verifyControllerMessage(_msgBuf, _msgLen, 7, _log);
	case OPCODE_READ_EVTLOG_ENABLE:
		return verifyControllerMessage(_msgBuf, _msgLen, 4, _log);
	}

	return FAIL;
}

int ComProxy::processWriteRequestResponse()
{
	if (_msgLen < 2)
	{
		return KEEP;
	}

	switch(_msgBuf[1])
	{
	case OPCODE_WRITE_EVTLOG_ENABLE:
	{
		if (_msgLen < 4)
		{
			return KEEP;
		}

		int res = verifyControllerMessage(_msgBuf, _msgLen, 4, _log);
		if (res > 0)
		{
			_connected = _msgBuf[2] != 0;
		}

		return res;
	}};

	return FAIL;
}

int ComProxy::processEventLogMessage()
{
    if (_msgBuf[0] == EVENT_LOG_ENTRY)
    {
        const int MessageSize = 3;

        if (_msgLen < MessageSize)
        {
            return KEEP;
        }

		int res = verifyControllerMessage(_msgBuf, _msgLen, MessageSize, _log);
		if (res > 0)
		{
			_hasEvent = true;
			_event.timestamp = millis();
       	 	_event.id = _msgBuf[1];
        	_event.data = 0;
		}

        return res;
    }
    else if (_msgBuf[0] == EVENT_LOG_DATA_ENTRY)
    {
        const int MessageSize = 5;

        if (_msgLen < MessageSize)
        {
            return KEEP;
        }

		int res = verifyControllerMessage(_msgBuf, _msgLen, MessageSize, _log);
		if (res > 0)
		{
			_hasEvent = true;
			_event.timestamp = millis();
       		_event.id = _msgBuf[1];
        	_event.data = _msgBuf[2] << 8 | _msgBuf[3];
		}

        return res;
    }

    return FAIL;
}
