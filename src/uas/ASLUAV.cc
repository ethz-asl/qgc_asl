#include "ASLUAV.h"
#include <iostream>
#include <QString>
#include "GAudioOutput.h"

ASLUAV::ASLUAV(MAVLinkProtocol* mavlink, int id)
	: UAS(mavlink, id), 
	emptyVoltage_ext(9.0), 
	warnVoltage_ext(10.5),
	fullVoltage_ext(12.5),
	tickVoltage_ext(warnVoltage_ext),
	lpVoltage_ext(0.0),
	tickLowpassVoltage_ext(0.0),
	lastTickVoltageValue_ext(0.0),
	startVoltage_ext(-1.0f)
{
}


ASLUAV::~ASLUAV(void)
{
}

//Overwritten function is in UAS
QString ASLUAV::getAutopilotTypeName()
{
    switch (autopilot)
    {
    case MAV_AUTOPILOT_ASLUAV:
        return "ASLUAV";
        break;
    default:
		return UAS::getAutopilotTypeName();
        break;
    }
}



void ASLUAV::receiveMessage(LinkInterface *link, mavlink_message_t message)
{
#ifdef MAVLINK_ENABLED_ASLUAV
	//std::cout << "msgid:"<<(int)message.msgid<<endl;

	if (!links.contains(link))
	{
		addLink(link);
		//        qDebug() << __FILE__ << __LINE__ << "ADDED LINK!" << link->getName();
	}

	if (!components.contains(message.compid))
	{
		QString componentName;

		switch (message.compid)
		{
		case MAV_COMP_ID_ALL:
		{
								componentName = "ANONYMOUS";
								break;
		}
		case MAV_COMP_ID_IMU:
		{
								componentName = "IMU #1";
								break;
		}
		case MAV_COMP_ID_CAMERA:
		{
								   componentName = "CAMERA";
								   break;
		}
		case MAV_COMP_ID_MISSIONPLANNER:
		{
										   componentName = "MISSIONPLANNER";
										   break;
		}
		}

		components.insert(message.compid, componentName);
		emit componentCreated(uasId, message.compid, componentName);
	}

	//    qDebug() << "UAS RECEIVED from" << message.sysid << "component" << message.compid << "msg id" << message.msgid << "seq no" << message.seq;

	// Only accept messages from this system (condition 1)
	// and only then if a) attitudeStamped is disabled OR b) attitudeStamped is enabled
	// and we already got one attitude packet
	if (message.sysid == uasId && (!attitudeStamped || (attitudeStamped && (lastAttitude != 0)) || message.msgid == MAVLINK_MSG_ID_ATTITUDE))
	{
		QString uasState;
		QString stateDescription;

		bool multiComponentSourceDetected = false;
		bool wrongComponent = false;

		switch (message.compid)
		{
		case MAV_COMP_ID_IMU_2:
			// Prefer IMU 2 over IMU 1 (FIXME)
			componentID[message.msgid] = MAV_COMP_ID_IMU_2;
			break;
		default:
			// Do nothing
			break;
		}

		// Store component ID
		if (componentID[message.msgid] == -1)
		{
			// Prefer the first component
			componentID[message.msgid] = message.compid;
		}
		else
		{
			// Got this message already
			if (componentID[message.msgid] != message.compid)
			{
				componentMulti[message.msgid] = true;
				wrongComponent = true;
			}
		}

		if (componentMulti[message.msgid] == true) multiComponentSourceDetected = true;

        //******************************************************************************
		//*** SWITCH CUSTOM ASLUAV-MESSAGES HERE
		//******************************************************************************
		switch (message.msgid)
        {
            case MAVLINK_MSG_ID_SENS_POWER:
			{
                //std::cout << "ASLUAV: Received SENS_POWER data message"<<endl;
				
                mavlink_sens_power_t data;
                mavlink_msg_sens_power_decode(&message, &data);

				// Battery charge/time remaining/voltage calculations
				lpVoltage_ext = data.adc121_vspb_volt;
				tickLowpassVoltage_ext = tickLowpassVoltage_ext*0.8f + 0.2f*data.adc121_vspb_volt;
				// We don't want to tick above the threshold
				if (tickLowpassVoltage_ext > tickVoltage_ext)
				{
					lastTickVoltageValue_ext = tickLowpassVoltage_ext;
				}
				if ((startVoltage_ext > 0.0f) && (tickLowpassVoltage_ext < tickVoltage_ext) && (fabs(lastTickVoltageValue_ext - tickLowpassVoltage_ext) > 0.1f)
					/* warn if lower than treshold */
					&& (lpVoltage_ext < tickVoltage_ext)
					/* warn only if we have at least the voltage of an empty LiPo cell, else we're sampling something wrong */
					&& (currentVoltage_ext > 3.3f)
					/* warn only if current voltage is really still lower by a reasonable amount */
					&& ((currentVoltage_ext - 0.2f) < tickVoltage_ext)
					/* warn only every 12 seconds */
					&& (QGC::groundTimeUsecs() - lastVoltageWarning) > 12000000)
				{
					GAudioOutput::instance()->say(QString("ADC121 Voltage warning for system %1: %2 volts").arg(getUASID()).arg(lpVoltage, 0, 'f', 1, QChar(' ')));
					lastVoltageWarning = QGC::groundTimeUsecs();
					lastTickVoltageValue_ext = tickLowpassVoltage_ext;
				}
				
				if (startVoltage_ext == -1.0f && currentVoltage > 0.1f) startVoltage_ext = currentVoltage_ext;

				emit PowerDataChanged(lpVoltage_ext, data.adc121_cspb_amp, data.adc121_cs1_amp, data.adc121_cs2_amp);
				//The rest of the message handling (i.e. adding data to the plots) is done elsewhere
				break;
			}
			case MAVLINK_MSG_ID_SENS_POWER_BOARD:
			{
				mavlink_sens_power_board_t data;
				mavlink_msg_sens_power_board_decode(&message, &data);
				float totalAmp = data.pwr_brd_aux_amp + data.pwr_brd_mot_l_amp + data.pwr_brd_mot_r_amp + data.pwr_brd_servo_1_amp + data.pwr_brd_servo_2_amp + data.pwr_brd_servo_3_amp + data.pwr_brd_servo_4_amp;
				emit PwrBrdStatChanged(data.pwr_brd_status);
				emit PowerDataChanged(data.pwr_brd_system_volt/1000, totalAmp, data.pwr_brd_mot_l_amp, data.pwr_brd_mot_r_amp);
				break;
			}
			case MAVLINK_MSG_ID_SENS_MPPT:
			{
				mavlink_sens_mppt_t data;
				mavlink_msg_sens_mppt_decode(&message, &data);
				emit MPPTDataChanged(data.mppt1_volt, data.mppt1_amp, data.mppt1_pwm, data.mppt1_status, data.mppt2_volt, data.mppt2_amp, data.mppt2_pwm, data.mppt2_status, data.mppt3_volt, data.mppt3_amp, data.mppt3_pwm, data.mppt3_status);
				break;
			}
			case MAVLINK_MSG_ID_SENS_BATMON:
			{
				mavlink_sens_batmon_t data;
				mavlink_msg_sens_batmon_decode(&message, &data);
				emit BatMonDataChanged(message.compid, data.voltage, data.current, data.SoC, data.temperature, data.batterystatus, data.hostfetcontrol, data.cellvoltage1, data.cellvoltage2, data.cellvoltage3, data.cellvoltage4, data.cellvoltage5, data.cellvoltage6);
				break;
			}
			case MAVLINK_MSG_ID_ASLCTRL_DATA:
			{
				mavlink_aslctrl_data_t data;
				mavlink_msg_aslctrl_data_decode(&message, &data);
								
				emit AslctrlDataChanged(data.uElev, data.uAil, data.uRud, data.uThrot, data.RollAngle,
					data.PitchAngle, data.YawAngle, data.RollAngleRef, data.PitchAngleRef, data.h);
				
				//receiveMessage(LinkInterface *link, mavlink_message_t message)
				
				//The rest of the message handling (i.e. adding data to the plots) is done elsewhere
				break;
			}
            case MAVLINK_MSG_ID_SENSORPOD_STATUS:
            {
                mavlink_sensorpod_status_t data;
                mavlink_msg_sensorpod_status_decode(&message, &data);
                emit SensorpodStatusChanged(data.visensor_rate_1, data.visensor_rate_2, data.visensor_rate_3, data.visensor_rate_4,
                                            data.recording_nodes_count,data.cpu_temp, data.free_space);

                break;
            }
			default:
			{
				//std::cout << "msgid:"<<message.(int)msgid<<endl; //"ASLUAV: Unknown message received"<<endl;
                UAS::receiveMessage(link, message);
				//Q_UNUSED(link);
				//Q_UNUSED(message);
				break;
			}
		}	
	}

#else
    // Let UAS handle the default message set
    UAS::receiveMessage(link, message);
    Q_UNUSED(link);
    Q_UNUSED(message);
#endif // MAVLINK_ENABLED_ASLUAV
}

void ASLUAV::setBatterySpecs(const QString& specs)
{
	if (specs.length() == 0) return;	

	batteryRemainingEstimateEnabled = false;
	QString stringList = specs;
	stringList = stringList.remove("V");
	stringList = stringList.remove("v");
	QStringList parts = stringList.split(",");

	bool ok;
	float temp;

	// Assign first battery spec value: PX4-internal voltage sensor
	if (parts.at(0).contains("%")) {
		QString percent = parts.at(0);
		percent = percent.remove("%");
		float temp = percent.toFloat(&ok);

		if (ok)	warnLevelPercent = temp;
		else emit textMessageReceived(0, 0, 0, "Could not set battery options, format is wrong");
	}
	else {
		temp = parts.at(0).toFloat(&ok);
		if (ok) tickVoltage = temp;
		else emit textMessageReceived(0, 0, 0, "Could not set battery options, format is wrong");
	}

	// Assign second battery spec value if available: External voltage sensor
	if (parts.length() > 1) {
		if (parts.at(1).contains("%")) {
			emit textMessageReceived(0, 0, 0, "External voltage sensor does not support relative warn levels, but requires absolute voltage warning limit!");
		}
		else {
			temp = parts.at(1).toFloat(&ok);
			if (ok) tickVoltage_ext = temp;
			else emit textMessageReceived(0, 0, 0, "Could not set battery options, format is wrong");
		}
	}
}

int ASLUAV::SendCommandLong(MAV_CMD CmdID, float param1, float param2, float param3, float param4, float param5, float param6, float param7)
{
	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, uasId, 0, CmdID, 1, param1, param2, param3, param4, param5, param6, param7);
	sendMessage(msg);

	std::cout << "ASLUAV: Command with ID #"<<CmdID<<" sent." << std::endl;
	return 0;
}

int ASLUAV::SendCommandLongTarget(MAV_CMD CmdID, uint8_t target_component, float param1, float param2, float param3, float param4, float param5, float param6, float param7)
{
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, uasId, target_component, CmdID, 1, param1, param2, param3, param4, param5, param6, param7);
    sendMessage(msg);

    std::cout << "ASLUAV: Command with ID #"<<CmdID<<" sent to component " << (int) target_component << "." << std::endl;
    return 0;
}
