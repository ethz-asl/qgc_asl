#ifndef SENSORPODSTATUS_H
#define SENSORPODSTATUS_H

#include <QObject>
#include <QWidget>
#include <stdint.h>
#include "QGCDockWidget.h"
#include "ui_SensorpodStatus.h"
#include "Vehicle.h"


namespace Ui {
class SensorpodStatus;
}

class QGraphicsScene;
class UASInterface;
class QTimer;
class UASInterface;

class SensorpodStatus : public QGCDockWidget
{
    Q_OBJECT

public:
    explicit SensorpodStatus(const QString& title, QAction* action, QWidget *parent = 0);
    ~SensorpodStatus();

protected:
    Ui::SensorpodStatus *ui;
	QGraphicsScene *m_scene;
    QTimer *m_UpdateReset;


protected slots:
    void updateSensorpodStatus(uint8_t rate1, uint8_t rate2, uint8_t rate3, uint8_t rate4, uint8_t numRecordTopics, uint8_t cpuTemp, uint16_t freeSpace);
    void setActiveUAS(void);
    void UpdateTimerTimeout(void);

    void PowerCycleSensorpodCmd(void);
};

#endif // SENSORPODSTATUS_H
