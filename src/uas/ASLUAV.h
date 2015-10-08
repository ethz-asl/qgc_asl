#ifndef _ASLUAV_H_
#define _ASLUAV_H_

#include "UAS.h"

class ASLUAV : public UAS
{
	Q_OBJECT
	Q_INTERFACES(UASInterface)

public:
	ASLUAV(void);
	ASLUAV(MAVLinkProtocol* mavlink, int id);
	~ASLUAV(void);

public:
    QString getAutopilotTypeName();	//Overwritten from UAS to add MAV_AUTOPILOT_ASLUAV type.
	int SendCommandLong(MAV_CMD CmdID, float param1 = 0.0f, float param2 = 0.0f, float param3 = 0.0f, float param4 = 0.0f, float param5 = 0.0f, float param6 = 0.0f, float param7 = 0.0f);
    int SendCommandLongTarget(MAV_CMD CmdID, uint8_t target_component, float param1 = 0.0f, float param2 = 0.0f, float param3 = 0.0f, float param4 = 0.0f, float param5 = 0.0f, float param6 = 0.0f, float param7 = 0.0f);
    double getThermalUpdraftVelocity(void);
    double getThermalStrength(void);
    double getThermalControlMode(void);

public slots:
    /** @brief Receive a MAVLink message from this MAV */
    void receiveMessage(LinkInterface* link, mavlink_message_t message);
	/** @brief Set Battery Specs for warnings, overwritten from UAS-class */
	void setBatterySpecs(const QString& specs);

signals:
	/** @brief CustomSensorData has changed */
	void AirspeedChanged(float airspeed);
	/** @brief CustomSensorData has changed */
    void AslctrlDataChanged(float uElev, float uAil, float uRud, float uThrot, float roll, float pitch, float yaw, float roll_ref, float pitch_ref, float h);
	void PowerDataChanged(float volt, float currpb, float curr_1, float curr_2);
	void PwrBrdStatChanged(uint8_t stat);
	void MPPTDataChanged(float volt1, float amp1, uint16_t pwm1, uint8_t status1, float volt2, float amp2, uint16_t pwm2, uint8_t status2, float volt3, float amp3, uint16_t pwm3, uint8_t status3);
	void BatMonDataChanged(uint8_t compid, uint16_t volt, int16_t current, uint8_t soc, float temp, uint16_t batStatus, uint16_t hostfetcontrol, uint16_t cellvolt1, uint16_t cellvolt2, uint16_t cellvolt3, uint16_t cellvolt4, uint16_t cellvolt5, uint16_t cellvolt6);
    void SensorpodStatusChanged(uint8_t rate1, uint8_t rate2, uint8_t rate3, uint8_t rate4, uint8_t numRecordTopics, uint8_t cpuTemp, uint16_t freeSpace);
    void ThermalUpdraftEstimateChanged(Waypoint *wp);

protected:
	// Add an external voltage sensor in addition to PX4-onboard sensor
	double currentVoltage_ext;	
	float lpVoltage_ext, tickLowpassVoltage_ext,lastTickVoltageValue_ext;
	float emptyVoltage_ext, warnVoltage_ext, fullVoltage_ext;
	float tickVoltage_ext;
	float startVoltage_ext;

	Waypoint ThermalUpdraftEstimate;
};

#endif //_ASLUAV_H_
