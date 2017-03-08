#ifndef WINDSENSOR_H
#define WINDSENSOR_H

#include "MultiVehicleDockWidget.h"
#include <QSerialPort>
#include "UAS.h"


class WindSensorStruct : public MultiVehicleDockWidget
{
    Q_OBJECT

public:
    explicit WindSensorStruct(const QString& title, QAction* action, QWidget *parent = 0);
    ~WindSensorStruct();

protected:
    // Override from MultiVehicleDockWidget
    virtual QWidget* _newVehicleWidget(Vehicle* vehicle, QWidget* parent);
};



namespace Ui {
class WindSensor;
}

class WindSensor : public QWidget
{
    Q_OBJECT

public:
    explicit WindSensor(Vehicle* vehicle, QWidget *parent = 0);
    ~WindSensor();

private slots:
    void received_data();
    void on_open_serial_clicked();

private:
    Vehicle* _vehicle;
    Ui::WindSensor *ui;
    QSerialPort serial_port;


};

#endif // WINDSENSOR_H
