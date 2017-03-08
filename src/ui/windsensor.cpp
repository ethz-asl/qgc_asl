#include "windsensor.h"
#include "ui_windsensor.h"
#include "QGCApplication.h"
#include "MAVLinkProtocol.h"

WindSensorStruct::WindSensorStruct(const QString& title, QAction* action, QWidget *parent)
    : MultiVehicleDockWidget(title, action, parent)
{
    init();

    loadSettings();
}

WindSensorStruct::~WindSensorStruct()
{

}

QWidget* WindSensorStruct::_newVehicleWidget(Vehicle* vehicle, QWidget* parent)
{
    return new WindSensor(vehicle, parent);
}



WindSensor::WindSensor(Vehicle* vehicle, QWidget *parent)
    : QWidget(parent),
      _vehicle(vehicle),
      ui(new Ui::WindSensor)
{
    ui->setupUi(this);

     QObject::connect(&serial_port, SIGNAL(readyRead()),this,SLOT(received_data()));
}


WindSensor::~WindSensor()
{
    serial_port.close();
    ui->textBrowser->clear();
    delete ui;
}


void WindSensor::received_data()
{
    QByteArray buffer;
    if (serial_port.canReadLine()) {
        qint64 bytes_available =  serial_port.bytesAvailable();
                buffer.resize(bytes_available);
                serial_port.readLine(buffer.data(), buffer.size());
                ui->textBrowser->clear();
                ui->textBrowser->setFontPointSize(12);
                QString wind_data = buffer.data();
                QStringList wind_data_list = wind_data.split(",");
                int time_ind = -1;
                for(int i = 0; i< wind_data_list.length();i++){
                    if(wind_data_list[i].contains(":"))
                    {
                        time_ind = i;
                        continue;
                    }
                                }
        if(time_ind !=-1){
              // Remove data from second sensor port (as we do not use a second sensor)
              wind_data_list.removeAt(7);
              wind_data_list.removeAt(7);
              wind_data_list.removeAt(7);
              QStringList wind_data_meaning_list;
              wind_data_meaning_list << "Time in 24h format" << "Average Wind Direction" << "Temperature in degrees Celsius" << "Average wind speed (m/s)" << "Maximum wind speed (m/s)" << "Standard deviation" << "Battery voltage" ;
              for(int i = time_ind; i< qMin(wind_data_list.length(),7 + time_ind);i++)  // iterate over the length of the input or maximum over 7 items, as we only care about the first seven entries
              ui->textBrowser->insertPlainText(QString("%1:  %2 \n").arg(wind_data_meaning_list[i-time_ind]).arg(wind_data_list[i]));

              // Send message to pixhawk
              mavlink_message_t msg;
              MAVLinkProtocol* mavlink = qgcApp()->toolbox()->mavlinkProtocol();
              mavlink_msg_wind_sensor_pack(mavlink->getSystemId(),mavlink->getComponentId(),&msg,wind_data_list[time_ind+1].toFloat(),wind_data_list[time_ind+3].toFloat(),wind_data_list[time_ind+4].toFloat(),wind_data_list[time_ind+5].toFloat());
              if(_vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg))
                   ui->textBrowser->insertPlainText(QString("sent succesfully"));
              else
                   ui->textBrowser->insertPlainText(QString("sending failed"));

        } else
             {ui->textBrowser->insertPlainText(QString("weird thing when reading data this is normal if you just started this up (number of bytes avialable: %1)").arg(bytes_available));
        }
    }




}




void WindSensor::on_open_serial_clicked()
{
    serial_port.close();
    QString input;
    input = ui->lineEdit->text();


    serial_port.setPortName(input);
    serial_port.setBaudRate(4800);
    if (!serial_port.open(QIODevice::ReadOnly)) {
            QString output = "Failed to open port\n";
            ui->textBrowser->insertPlainText(output);
        }
    else
        ui->textBrowser->insertPlainText("Succesfully connected to Serial port \n");
}

