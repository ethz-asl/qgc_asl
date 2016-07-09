#include "sensorpodstatus.h"
#include "ui_sensorpodstatus.h"
#include <qgraphicsscene.h>
#include <QMessageBox>
#include <QTimer>
#include <math.h>
#include <qdebug.h>
#include "../uas/ASLUAV.h"
#include "UASInterface.h"
#include "UASManager.h"
#include "QGCApplication.h"

#define RESETTIMEMS 10000

SensorpodStatus::SensorpodStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorpodStatus),
    m_scene(new QGraphicsScene(this)),
    m_UpdateReset(new QTimer (this))
{
    ui->setupUi(this);
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));
    connect(ui->PowerCycleButton, SIGNAL(clicked()), this, SLOT(PowerCycleSensorpodCmd()));
//	connect(qgcApp(), SIGNAL(styleChanged(bool)), this, SLOT(styleChanged(bool)));
    m_UpdateReset->setInterval(RESETTIMEMS);
    m_UpdateReset->setSingleShot(false);
    connect(m_UpdateReset, SIGNAL(timeout()), this, SLOT(UpdateTimerTimeout()));
    if (UASManager::instance()->getActiveUAS())
    {
        setActiveUAS(UASManager::instance()->getActiveUAS());
    }
    m_UpdateReset->start();
}

SensorpodStatus::~SensorpodStatus()
{
    delete ui;
	delete m_scene;
    delete m_UpdateReset;
}

void SensorpodStatus::updateSensorpodStatus(uint8_t rate1, uint8_t rate2, uint8_t rate3, uint8_t rate4, uint8_t numRecordTopics, uint8_t cpuTemp, uint16_t freeSpace)
{
    m_UpdateReset->stop();
    ui->topic1rate->setText(QString("%1 Hz").arg(rate1));
    ui->topic2rate->setText(QString("%1 Hz").arg(rate2));
    ui->topic3rate->setText(QString("%1 Hz").arg(rate3));
    ui->topic4rate->setText(QString("%1 Hz").arg(rate4));
    ui->numRecNodes->setText(QString("%1").arg(numRecordTopics));
    ui->cpuTemp->setText(QString("%1 °C").arg(cpuTemp));
    ui->freeDiskSpace->setText(QString("%1 GB").arg(freeSpace/100.0));
    m_UpdateReset->start(RESETTIMEMS);
}


//void EnergyBudget::resizeEvent(QResizeEvent *event)
//{
//	QWidget::resizeEvent(event);
//	ui->overviewGraphicsView->fitInView(m_scene->sceneRect(), Qt::AspectRatioMode::KeepAspectRatio);
//}

void SensorpodStatus::setActiveUAS(UASInterface *uas)
{
	//disconnect any previous uas
    disconnect(this, SLOT(updateSensorpodStatus(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t)));

    //connect the uas if asluas
	ASLUAV *asluas = dynamic_cast<ASLUAV*>(uas);
	if (asluas)
	{
        connect(asluas, SIGNAL(SensorpodStatusChanged(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t)), this, SLOT(updateSensorpodStatus(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t)));
	}
	//else set to standard output
	else
	{

	}
}

void SensorpodStatus::PowerCycleSensorpodCmd()
{
	QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Payload control"), tr("Sending command to control payload. Use this with caution! Are you sure?"), QMessageBox::No | QMessageBox::Yes);

	if (reply == QMessageBox::Yes) {
		//Send the message via the currently active UAS
		ASLUAV *tempUAS = (ASLUAV*) UASManager::instance()->getActiveUAS();
        if (tempUAS) {
            float cmd1 = 0.0f, cmd2 = 0.0f;

            if(ui->powerOn1->isChecked()) cmd1 = 0.5f;
            else if(ui->powerOff1->isChecked()) cmd1 = -0.5f;
            else if(ui->powerCycle1->isChecked()) cmd1 = 1.0f;

            if(ui->powerOn2->isChecked()) cmd2 = 0.5f;
            else if(ui->powerOff2->isChecked()) cmd2 = -0.5f;

            tempUAS->SendCommandLong(MAV_CMD_PAYLOAD_CONTROL, cmd1, cmd2);

            ui->noCommand1->setChecked(true);
            ui->noCommand2->setChecked(true);
		}
	}

}

void SensorpodStatus::UpdateTimerTimeout()
{
    ui->topic1rate->setText(QString("--"));
    ui->topic2rate->setText(QString("--"));
    ui->topic3rate->setText(QString("--"));
    ui->topic4rate->setText(QString("--"));
    ui->numRecNodes->setText(QString("--"));
    ui->cpuTemp->setText(QString("--"));
    ui->freeDiskSpace->setText(QString("--"));
}
