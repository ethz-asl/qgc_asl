#include "EnergyBudget.h"
#include "ui_EnergyBudget.h"
#include <qgraphicsscene.h>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QMessageBox>
#include <QTimer>
#include <math.h>
#include <qdebug.h>
#include "UASInterface.h"
#include "MultiVehicleManager.h"
#include "QGCApplication.h"
#include "SettingsManager.h"

#define OVERVIEWTOIMAGEHEIGHTSCALE (3.0)
#define OVERVIEWTOIMAGEWIDTHSCALE (3.0)

#define CELLPOWERHYSTMIN 1
#define CELLPOWERHYSTMAX 3
#define BATPOWERHIGHMIN 1
#define BATPOWERHIGHMAX 3
#define BATPOWERLOWMIN -3
#define BATPOWERLOWMAX -1
#define MPPTRESETTIMEMS 3000
#define BATMONLEDDIAMETER 3

class Hysteresisf
{
public:
	Hysteresisf(float minVal, float maxVal, bool isHighState);
	bool check(float);
private:
	float const m_minVal;
	float const m_maxVal;
    bool m_highState;
};

EnergyBudget::EnergyBudget(const QString &title, QAction *action, QWidget *parent) :
QGCDockWidget(title, action, parent),
ui(new Ui::EnergyBudget),
m_scene(new QGraphicsScene(this)),
m_propPixmap(m_scene->addPixmap(QPixmap(":/qmlimages/EnergyBudget/Prop"))),
m_cellPixmap(m_scene->addPixmap(QPixmap(":/qmlimages/EnergyBudget/Solarcell"))),
m_batPixmap(m_scene->addPixmap(QPixmap(":/qmlimages/EnergyBudget/Battery"))),
m_chargePath(m_scene->addPath(QPainterPath())),
m_cellToPropPath(m_scene->addPath(QPainterPath())),
m_batToPropPath(m_scene->addPath(QPainterPath())),
m_chargePowerText(m_scene->addText(QString("???"))),
m_cellPowerText(m_scene->addText(QString("???"))),
m_BckpBatText(m_scene->addText(QString("Backup Battery in use!!!"))),
m_cellUsePowerText(m_scene->addText(QString("???"))),
m_batUsePowerText(m_scene->addText(QString("???"))),
m_SystemUsePowerText(m_scene->addText(QString("???"))),
m_lastBckpBatWarn(0),
m_cellPower(0.0),
m_batUsePower(0.0),
m_propUsePower(0.0),
m_chargePower(0.0),
m_thrust(0.0),
m_batCharging(batChargeStatus::CHRG),
m_batHystHigh(new Hysteresisf(BATPOWERHIGHMIN, BATPOWERHIGHMAX, true)),
m_batHystLow(new Hysteresisf(BATPOWERLOWMIN, BATPOWERLOWMAX, true)),
m_mpptHyst(new Hysteresisf(CELLPOWERHYSTMIN, CELLPOWERHYSTMAX, true)),
m_MPPTUpdateReset(new QTimer(this))
{
    ui->setupUi(this);
    ui->overviewGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->overviewGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->buildGraphicsImage();
    ui->overviewGraphicsView->setScene(m_scene);
    ui->overviewGraphicsView->fitInView(QRectF(0,0,150,150), Qt::AspectRatioMode::KeepAspectRatio);
    //ui->overviewGraphicsView->fitInView(m_scene->sceneRect(), Qt::AspectRatioMode::KeepAspectRatio);
    connect(qgcApp()->toolbox()->multiVehicleManager(), &MultiVehicleManager::vehicleAdded, this, &EnergyBudget::setActiveUAS);

    // set LED size
    ui->bat1FCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2FCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3FCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1FDLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2FDLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3FDLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1COVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2COVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3COVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1CUVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2CUVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3CUVLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1OTCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2OTCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3OTCLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1DSGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2DSGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3DSGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1CHGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2CHGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3CHGLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat1PFLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat2PFLed->setDiameter(BATMONLEDDIAMETER);
    ui->bat3PFLed->setDiameter(BATMONLEDDIAMETER);

    // set LED color
    ui->bat1FCLed->setColor(QColor(Qt::green));
    ui->bat2FCLed->setColor(QColor(Qt::green));
    ui->bat3FCLed->setColor(QColor(Qt::green));
    ui->bat1FDLed->setColor(QColor(Qt::red));
    ui->bat2FDLed->setColor(QColor(Qt::red));
    ui->bat3FDLed->setColor(QColor(Qt::red));
    ui->bat1COVLed->setColor(QColor(Qt::red));
    ui->bat2COVLed->setColor(QColor(Qt::red));
    ui->bat3COVLed->setColor(QColor(Qt::red));
    ui->bat1CUVLed->setColor(QColor(Qt::red));
    ui->bat2CUVLed->setColor(QColor(Qt::red));
    ui->bat3CUVLed->setColor(QColor(Qt::red));
    ui->bat1OTCLed->setColor(QColor(Qt::red));
    ui->bat2OTCLed->setColor(QColor(Qt::red));
    ui->bat3OTCLed->setColor(QColor(Qt::red));
    ui->bat1DSGLed->setColor(QColor(Qt::yellow));
    ui->bat2DSGLed->setColor(QColor(Qt::yellow));
    ui->bat3DSGLed->setColor(QColor(Qt::yellow));
    ui->bat1CHGLed->setColor(QColor(Qt::yellow));
    ui->bat2CHGLed->setColor(QColor(Qt::yellow));
    ui->bat3CHGLed->setColor(QColor(Qt::yellow));
    ui->bat1PFLed->setColor(QColor(Qt::red));
    ui->bat2PFLed->setColor(QColor(Qt::red));
    ui->bat3PFLed->setColor(QColor(Qt::red));
    ui->bat1PFLed->setState(false);
    ui->bat2PFLed->setState(false);
    ui->bat3PFLed->setState(false);

	connect(ui->ResetMPPTButton, SIGNAL(clicked()), this, SLOT(ResetMPPTCmd()));
    //connect(qgcApp(), SIGNAL(styleChanged(bool)), this, SLOT(styleChanged(bool)));
	m_MPPTUpdateReset->setInterval(MPPTRESETTIMEMS);
	m_MPPTUpdateReset->setSingleShot(false);
	connect(m_MPPTUpdateReset, SIGNAL(timeout()), this, SLOT(MPPTTimerTimeout()));
	ui->ResetMPPTEdit->setValidator(new QIntValidator(this));
    if (qgcApp()->toolbox()->multiVehicleManager()->activeVehicle())
    {
        setActiveUAS(qgcApp()->toolbox()->multiVehicleManager()->activeVehicle());
    }
    m_MPPTUpdateReset->start();
}

EnergyBudget::~EnergyBudget()
{
    delete ui;
	delete m_scene;
	delete m_MPPTUpdateReset;
	delete m_batHystHigh;
	delete m_batHystLow;
	delete m_mpptHyst;
}

void EnergyBudget::buildGraphicsImage()
{
    m_scene->setSceneRect(0.0, 0.0, 1000.0, 1000.0);
    qreal penWidth(40);
	// scale and place images

    QRectF tempRectF=m_propPixmap->boundingRect();
    m_propPixmap->setScale(this->adjustImageScale(m_scene->sceneRect(), tempRectF ));
    tempRectF=m_cellPixmap->boundingRect();
    m_cellPixmap->setScale(this->adjustImageScale(m_scene->sceneRect(), tempRectF ));
    tempRectF=m_batPixmap->boundingRect();
    m_batPixmap->setScale(this->adjustImageScale(m_scene->sceneRect(), tempRectF ));
	double maxheight(std::fmax(m_propPixmap->mapRectToScene(m_propPixmap->boundingRect()).height(), m_batPixmap->mapRectToScene(m_batPixmap->boundingRect()).height()));
	m_propPixmap->setOffset(0.0, m_propPixmap->mapRectFromScene(m_scene->sceneRect()).height() - m_propPixmap->mapRectFromScene(QRectF(0.0,0.0,1.0,maxheight)).height()/2.0 - m_propPixmap->boundingRect().height() / 2.0);
	m_batPixmap->setOffset(m_batPixmap->mapRectFromScene(m_scene->sceneRect()).width() - m_batPixmap->boundingRect().width(), m_batPixmap->mapRectFromScene(m_scene->sceneRect()).height() - m_batPixmap->mapRectFromScene(QRectF(0.0, 0.0, 1.0, maxheight)).height() / 2.0 - m_batPixmap->boundingRect().height() / 2.0);
	m_cellPixmap->setOffset(m_cellPixmap->mapRectFromScene(m_scene->sceneRect()).width() / 2.0, 0.0);
	// Add arrows between images
	QRectF cellRect(m_cellPixmap->sceneBoundingRect());
	QRectF propRect(m_propPixmap->sceneBoundingRect());
	QRectF batRect(m_batPixmap->sceneBoundingRect());
	QPainterPath cellPath(QPointF(cellRect.x() + cellRect.width(), cellRect.y() + cellRect.height() / 2.0));
	cellPath.lineTo(batRect.x()+batRect.width()/2.0, cellRect.y() + cellRect.height() / 2.0);
	cellPath.lineTo(batRect.x() + batRect.width() / 2.0, batRect.y());
	m_chargePath->setPen(QPen(QBrush(Qt::GlobalColor::green, Qt::BrushStyle::SolidPattern), penWidth));
	m_chargePath->setPath(m_chargePath->mapFromScene(cellPath));
	QPainterPath cell2PropPath(QPointF(cellRect.x(), cellRect.y() + cellRect.height() / 2.0));
	cell2PropPath.lineTo(propRect.x()+propRect.width()/2.0, cellRect.y() + cellRect.height() / 2.0);
	cell2PropPath.lineTo(propRect.x() + propRect.width()/2.0, propRect.y());
	m_cellToPropPath->setPen(QPen(QBrush(Qt::GlobalColor::green, Qt::BrushStyle::SolidPattern), penWidth));
	m_cellToPropPath->setPath(m_cellToPropPath->mapFromScene(cell2PropPath));
	QPainterPath bat2PropPath(QPointF(batRect.x(), batRect.y() + batRect.height() / 2.0));
	bat2PropPath.lineTo(propRect.x()+propRect.width(),propRect.y() + propRect.height()/2.0);
	m_batToPropPath->setPen(QPen(QBrush(Qt::GlobalColor::red, Qt::BrushStyle::SolidPattern), penWidth));
	m_batToPropPath->setPath(m_batToPropPath->mapFromScene(bat2PropPath));
	// Add text
	m_chargePowerText->setPos(batRect.x() + batRect.width() / 2.0 - 5*penWidth, (batRect.y() + cellRect.y()+cellRect.height())/ 2.0);
	m_chargePowerText->setFont(QFont("Helvetica", penWidth*1.2));
	m_batUsePowerText->setPos((batRect.x() + propRect.x()) / 2.0, batRect.y() + batRect.height() / 2.0 - 3*penWidth);
	m_batUsePowerText->setFont(QFont("Helvetica", penWidth*1.2));
	m_cellPowerText->setPos(cellRect.x()+cellRect.width()/2.0, cellRect.y() - 2*penWidth);
	m_cellPowerText->setFont(QFont("Helvetica", penWidth*1.2));
	m_cellUsePowerText->setPos(propRect.x() + propRect.width() / 2.0 + 2*penWidth, (propRect.y() + cellRect.y() + cellRect.height()) / 2.0);
	m_cellUsePowerText->setFont(QFont("Helvetica", penWidth*1.2));
	m_SystemUsePowerText->setPos(propRect.x() + propRect.width()/2.0 -1.0*penWidth, propRect.y() + propRect.height());
	m_SystemUsePowerText->setFont(QFont("Helvetica", penWidth*1.2));
	m_BckpBatText->setPos(propRect.x(), propRect.y());
	m_BckpBatText->setFont(QFont("Helvetica", penWidth * 2));
	m_BckpBatText->setVisible(false);

    // set the right textcolor
    styleChanged(qgcApp()->toolbox()->settingsManager()->appSettings()->indoorPalette()->rawValue().toBool());

	// Recalc scene bounding rect
    m_scene->setSceneRect(QRectF(0.0, 0.0, 0.0, 0.0));
}

qreal EnergyBudget::adjustImageScale(const QRectF &viewSize, QRectF &img)
{
	return std::fmin((viewSize.height() / (OVERVIEWTOIMAGEHEIGHTSCALE*img.height())), (viewSize.width() / (OVERVIEWTOIMAGEWIDTHSCALE*img.width())));
}

void EnergyBudget::updatePower(float volt, float currpb, float curr_1, float curr_2)
{
	m_propUsePower = volt*currpb;
	m_SystemUsePowerText->setPlainText(QString("%1W @Thr %2%").arg(m_propUsePower, 0, 'f', 1).arg(m_thrust*100.0,0,'f',0));
	ui->powerA1Value->setText(QString("%1").arg(curr_1));
	ui->powerA2Value->setText(QString("%1").arg(curr_2));
	ui->powerAValue->setText(QString("%1").arg(currpb));
	ui->powerVValue->setText(QString("%1").arg(volt));

	updateGraphicsImage();
}

void EnergyBudget::updatePowerHL(uint8_t p_out)
{
    m_propUsePower = p_out * 2;
    m_SystemUsePowerText->setPlainText(QString("%1W @Thr %2%").arg(m_propUsePower, 0, 'f', 1).arg(m_thrust*100.0,0,'f',0));
    ui->powerA1Value->setText(QString("N/A"));
    ui->powerA2Value->setText(QString("N/A"));
    ui->powerAValue->setText(QString("N/A"));
    ui->powerVValue->setText(QString("N/A"));

    updateGraphicsImage();
}

void EnergyBudget::onSensPowerBoardChanged(uint8_t pwr_brd_status)
{
#define BCKPBATREG 0x20
    if (pwr_brd_status & BCKPBATREG)
    {
        m_scene->setBackgroundBrush(Qt::red);
        m_BckpBatText->setVisible(true);
        if ((QGC::groundTimeUsecs() - m_lastBckpBatWarn) > 10000000)
        {
            m_lastBckpBatWarn = QGC::groundTimeUsecs();
            qgcApp()->toolbox()->audioOutput()->say("Critical, System switched to backup battery!");
        }
    }
    else
    {
        m_scene->setBackgroundBrush(Qt::NoBrush);
        m_BckpBatText->setVisible(false);
    }

}

void EnergyBudget::updateBatMon(uint8_t compid, uint16_t volt, int16_t current, uint8_t soc, float temp, uint16_t batStatus, uint32_t batSafety, uint32_t batOperation, uint8_t batmonStatusByte, uint16_t cellvolt1, uint16_t cellvolt2, uint16_t cellvolt3, uint16_t cellvolt4, uint16_t cellvolt5, uint16_t cellvolt6)
{
	switch (compid)
	{
    case LEFTBATMONCOMPID:
			ui->bat1VLabel->setText(QString("%1").arg(volt/1000.0));
			ui->bat1VLabel_2->setText(QString("%1").arg(volt / 1000.0));
			ui->bat1ALabel->setText(QString("%1").arg(current / 1000.0));
			ui->bat1PowerLabel->setText(QString("%1").arg((volt / 1000.0)*(current / 1000.0)));
			ui->bat1CurrentLabel_2->setText(QString("%1").arg((current / 1000.0)));
            ui->bat1StatLabel->setText(convertBatteryStatus(batStatus));
            ui->bat1SafetyLabel->setText(QString("%1").arg(batSafety));
            ui->bat1OperLabel->setText(QString("%1").arg(batOperation));
			ui->bat1TempLabel->setText(QString("%1").arg(temp));
			ui->bat1SoCBar->setValue(soc);
			ui->bat1Cell1Label->setText(QString("%1").arg(cellvolt1));
			ui->bat1Cell2Label->setText(QString("%1").arg(cellvolt2));
			ui->bat1Cell3Label->setText(QString("%1").arg(cellvolt3));
			ui->bat1Cell4Label->setText(QString("%1").arg(cellvolt4));
			ui->bat1Cell5Label->setText(QString("%1").arg(cellvolt5));
			ui->bat1Cell6Label->setText(QString("%1").arg(cellvolt6));
            ui->bat1FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat1FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat1COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat1CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat1OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat1DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat1CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat1PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
			break;

    case CENTERBATMONCOMPID:
			ui->bat2VLabel->setText(QString("%1").arg(volt / 1000.0));
			ui->bat2VLabel_2->setText(QString("%1").arg(volt / 1000.0));
			ui->bat2ALabel->setText(QString("%1").arg(current / 1000.0));
			ui->bat2PowerLabel->setText(QString("%1").arg((volt / 1000.0)*(current / 1000.0)));
			ui->bat2CurrentLabel_2->setText(QString("%1").arg((current / 1000.0)));
            ui->bat2StatLabel->setText(convertBatteryStatus(batStatus));
            ui->bat2SafetyLabel->setText(QString("%1").arg(batSafety));
            ui->bat2OperLabel->setText(QString("%1").arg(batOperation));
			ui->bat2TempLabel->setText(QString("%1").arg(temp));
			ui->bat2SoCBar->setValue(soc);
			ui->bat2Cell1Label->setText(QString("%1").arg(cellvolt1));
			ui->bat2Cell2Label->setText(QString("%1").arg(cellvolt2));
			ui->bat2Cell3Label->setText(QString("%1").arg(cellvolt3));
			ui->bat2Cell4Label->setText(QString("%1").arg(cellvolt4));
			ui->bat2Cell5Label->setText(QString("%1").arg(cellvolt5));
			ui->bat2Cell6Label->setText(QString("%1").arg(cellvolt6));
            ui->bat2FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat2FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat2COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat2CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat2OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat2DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat2CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat2PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
			break;

    case RIGHTBATMONCOMPID:
			ui->bat3VLabel->setText(QString("%1").arg(volt / 1000.0));
			ui->bat3VLabel_2->setText(QString("%1").arg(volt / 1000.0));
			ui->bat3ALabel->setText(QString("%1").arg(current / 1000.0));
			ui->bat3PowerLabel->setText(QString("%1").arg((volt / 1000.0)*(current / 1000.0)));
			ui->bat3CurrentLabel_2->setText(QString("%1").arg((current / 1000.0)));
            ui->bat3StatLabel->setText(convertBatteryStatus(batStatus));
            ui->bat3SafetyLabel->setText(QString("%1").arg(batSafety));
            ui->bat3OperLabel->setText(QString("%1").arg(batOperation));
			ui->bat3TempLabel->setText(QString("%1").arg(temp));
			ui->bat3SoCBar->setValue(soc);
			ui->bat3Cell1Label->setText(QString("%1").arg(cellvolt1));
			ui->bat3Cell2Label->setText(QString("%1").arg(cellvolt2));
			ui->bat3Cell3Label->setText(QString("%1").arg(cellvolt3));
			ui->bat3Cell4Label->setText(QString("%1").arg(cellvolt4));
			ui->bat3Cell5Label->setText(QString("%1").arg(cellvolt5));
			ui->bat3Cell6Label->setText(QString("%1").arg(cellvolt6));
            ui->bat3FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat3FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat3COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat3CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat3OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat3DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat3CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat3PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
			break;
	}

	float power((ui->bat1PowerLabel->text().toDouble() + ui->bat2PowerLabel->text().toDouble() + ui->bat3PowerLabel->text().toDouble()));
	if (m_batHystHigh->check(power))
	{
		m_batCharging = batChargeStatus::CHRG;
		m_chargePower = power;
		m_batUsePower = 0;
	}
	else if(!(m_batHystLow->check(power)))
	{
		m_batCharging = batChargeStatus::DSCHRG;
		m_chargePower = 0;
		m_batUsePower = -power;
	}
	else
	{
		m_batCharging = batChargeStatus::LVL;
		m_chargePower = 0;
		m_batUsePower = 0;
	}
	updateGraphicsImage();
}

void EnergyBudget::updateBatMonHL(uint8_t compid, uint8_t volt, int8_t p_avg, uint8_t batmonStatusByte)
{
    switch (compid)
    {
    case LEFTBATMONCOMPID:
            ui->bat1VLabel->setText(QString("%1").arg(volt / 10.0));
            ui->bat1VLabel_2->setText(QString("%1").arg(volt / 10.0));
            ui->bat1ALabel->setText(QString("N/A"));
            ui->bat1PowerLabel->setText(QString("%1").arg(p_avg * 4 / 3.0));
            ui->bat1CurrentLabel_2->setText(QString("N/A"));
            ui->bat1StatLabel->setText(QString("N/A"));
            ui->bat1SafetyLabel->setText(QString("N/A"));
            ui->bat1OperLabel->setText(QString("N/A"));
            ui->bat1TempLabel->setText(QString("N/A"));
            ui->bat1SoCBar->setValue(QString("N/A").toInt());
            ui->bat1Cell1Label->setText(QString("N/A"));
            ui->bat1Cell2Label->setText(QString("N/A"));
            ui->bat1Cell3Label->setText(QString("N/A"));
            ui->bat1Cell4Label->setText(QString("N/A"));
            ui->bat1Cell5Label->setText(QString("N/A"));
            ui->bat1Cell6Label->setText(QString("N/A"));
            ui->bat1FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat1FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat1COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat1CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat1OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat1DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat1CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat1PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
            break;

    case CENTERBATMONCOMPID:
            ui->bat2VLabel->setText(QString("%1").arg(volt / 10.0));
            ui->bat2VLabel_2->setText(QString("%1").arg(volt / 10.0));
            ui->bat2ALabel->setText(QString("N/A"));
            ui->bat2PowerLabel->setText(QString("%1").arg(p_avg * 4 / 3.0));
            ui->bat2CurrentLabel_2->setText(QString("N/A"));
            ui->bat2StatLabel->setText(QString("N/A"));
            ui->bat2SafetyLabel->setText(QString("N/A"));
            ui->bat2OperLabel->setText(QString("N/A"));
            ui->bat2TempLabel->setText(QString("N/A"));
            ui->bat2SoCBar->setValue(QString("N/A").toInt());
            ui->bat2Cell1Label->setText(QString("N/A"));
            ui->bat2Cell2Label->setText(QString("N/A"));
            ui->bat2Cell3Label->setText(QString("N/A"));
            ui->bat2Cell4Label->setText(QString("N/A"));
            ui->bat2Cell5Label->setText(QString("N/A"));
            ui->bat2Cell6Label->setText(QString("N/A"));
            ui->bat2FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat2FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat2COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat2CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat2OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat2DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat2CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat2PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
            break;

    case RIGHTBATMONCOMPID:
            ui->bat3VLabel->setText(QString("%1").arg(volt / 10.0));
            ui->bat3VLabel_2->setText(QString("%1").arg(volt / 10.0));
            ui->bat3ALabel->setText(QString("N/A"));
            ui->bat3PowerLabel->setText(QString("%1").arg(p_avg * 4 / 3.0));
            ui->bat3CurrentLabel_2->setText(QString("N/A"));
            ui->bat3StatLabel->setText(QString("N/A"));
            ui->bat3SafetyLabel->setText(QString("N/A"));
            ui->bat3OperLabel->setText(QString("N/A"));
            ui->bat3TempLabel->setText(QString("N/A"));
            ui->bat3SoCBar->setValue(QString("N/A").toInt());
            ui->bat3Cell1Label->setText(QString("N/A"));
            ui->bat3Cell2Label->setText(QString("N/A"));
            ui->bat3Cell3Label->setText(QString("N/A"));
            ui->bat3Cell4Label->setText(QString("N/A"));
            ui->bat3Cell5Label->setText(QString("N/A"));
            ui->bat3Cell6Label->setText(QString("N/A"));
            ui->bat3FCLed->setState((bool)(0x1 & (batmonStatusByte >> 0)));
            ui->bat3FDLed->setState((bool)(0x1 & (batmonStatusByte >> 1)));
            ui->bat3COVLed->setState((bool)(0x1 & (batmonStatusByte >> 2)));
            ui->bat3CUVLed->setState((bool)(0x1 & (batmonStatusByte >> 3)));
            ui->bat3OTCLed->setState((bool)(0x1 & (batmonStatusByte >> 4)));
            ui->bat3DSGLed->setState((bool)(0x1 & (batmonStatusByte >> 5)));
            ui->bat3CHGLed->setState((bool)(0x1 & (batmonStatusByte >> 6)));
            ui->bat3PFLed->setFlashing((bool)(0x1 & (batmonStatusByte >> 7)));
            break;
    }

    float power((ui->bat1PowerLabel->text().toDouble() + ui->bat2PowerLabel->text().toDouble() + ui->bat3PowerLabel->text().toDouble()));
    if (m_batHystHigh->check(power))
    {
        m_batCharging = batChargeStatus::CHRG;
        m_chargePower = power;
        m_batUsePower = 0;
    }
    else if(!(m_batHystLow->check(power)))
    {
        m_batCharging = batChargeStatus::DSCHRG;
        m_chargePower = 0;
        m_batUsePower = -power;
    }
    else
    {
        m_batCharging = batChargeStatus::LVL;
        m_chargePower = 0;
        m_batUsePower = 0;
    }
    updateGraphicsImage();
}



void EnergyBudget::updateMPPT(float volt1, float amp1, uint16_t pwm1, uint8_t status1, float volt2, float amp2, uint16_t pwm2, uint8_t status2, float volt3, float amp3, uint16_t pwm3, uint8_t status3)
{
	m_MPPTUpdateReset->stop();
	ui->mppt1VLabel->setText(QString("%1").arg(volt1));
	ui->mppt2VLabel->setText(QString("%1").arg(volt2));
	ui->mppt3VLabel->setText(QString("%1").arg(volt3));
	ui->mppt1ALabel->setText(QString("%1").arg(amp1));
	ui->mppt2ALabel->setText(QString("%1").arg(amp2));
	ui->mppt3ALabel->setText(QString("%1").arg(amp3));
	ui->mppt1PowerLabel->setText(QString("%1").arg(volt1*amp1));
	ui->mppt2PowerLabel->setText(QString("%1").arg(volt2*amp2));
	ui->mppt3PowerLabel->setText(QString("%1").arg(volt3*amp3));
	ui->mppt1ModLabel->setText(convertMPPTModus(status1));
	ui->mppt2ModLabel->setText(convertMPPTModus(status2));
	ui->mppt3ModLabel->setText(convertMPPTModus(status3));
	ui->mppt1StatLabel->setText(convertMPPTModus(status1));
	ui->mppt2StatLabel->setText(convertMPPTModus(status2));
	ui->mppt3StatLabel->setText(convertMPPTModus(status3));
	ui->mppt1PwmLabel->setText(QString("%1").arg(pwm1));
	ui->mppt2PwmLabel->setText(QString("%1").arg(pwm2));
	ui->mppt3PwmLabel->setText(QString("%1").arg(pwm3));

	m_cellPower = (volt1*amp1 + volt2*amp2 + volt3*amp3);
	updateGraphicsImage();
	m_MPPTUpdateReset->start(MPPTRESETTIMEMS);
}

void EnergyBudget::updateMPPTHL(uint8_t volt1, uint8_t volt2, uint8_t volt3)
{
    m_MPPTUpdateReset->stop();
    ui->mppt1VLabel->setText(QString("%1").arg(volt1 / 10.0));
    ui->mppt2VLabel->setText(QString("%1").arg(volt2 / 10.0));
    ui->mppt3VLabel->setText(QString("%1").arg(volt3 / 10.0));
    ui->mppt1ALabel->setText(QString("N/A"));
    ui->mppt2ALabel->setText(QString("N/A"));
    ui->mppt3ALabel->setText(QString("N/A"));
    ui->mppt1PowerLabel->setText(QString("N/A"));
    ui->mppt2PowerLabel->setText(QString("N/A"));
    ui->mppt3PowerLabel->setText(QString("N/A"));
    ui->mppt1ModLabel->setText(QString("N/A"));
    ui->mppt2ModLabel->setText(QString("N/A"));
    ui->mppt3ModLabel->setText(QString("N/A"));
    ui->mppt1StatLabel->setText(QString("N/A"));
    ui->mppt2StatLabel->setText(QString("N/A"));
    ui->mppt3StatLabel->setText(QString("N/A"));
    ui->mppt1PwmLabel->setText(QString("N/A"));
    ui->mppt2PwmLabel->setText(QString("N/A"));
    ui->mppt3PwmLabel->setText(QString("N/A"));

    m_cellPower = (QString("N/A").toFloat());
    updateGraphicsImage();
    m_MPPTUpdateReset->start(MPPTRESETTIMEMS);
}

void EnergyBudget::updateGraphicsImage(void)
{
	if (m_batCharging == batChargeStatus::CHRG)
	{
		m_chargePath->setVisible(true);
		m_chargePowerText->setVisible(true);
		m_chargePowerText->setPlainText(QString("%1W").arg(m_chargePower, 0, 'f', 1));
		m_batToPropPath->setVisible(false);
		m_batUsePowerText->setVisible(false);
	}
	else if (m_batCharging == batChargeStatus::DSCHRG)
	{
		m_chargePath->setVisible(false);
		m_chargePowerText->setVisible(false);
		m_batToPropPath->setVisible(true);
		m_batUsePowerText->setVisible(true);
		m_batUsePowerText->setPlainText(QString("%1W").arg(m_batUsePower, 0, 'f', 1));
	}
	else
	{
		m_chargePath->setVisible(false);
		m_chargePowerText->setVisible(false);
		m_batToPropPath->setVisible(false);
		m_batUsePowerText->setVisible(false);
	}
	if (m_mpptHyst->check(m_cellPower))
	{
		m_cellToPropPath->setVisible(true);
		m_cellPowerText->setPlainText(QString("%1W").arg(m_cellPower, 0, 'f', 1));
		m_cellUsePowerText->setVisible(true);
		m_cellUsePowerText->setPlainText(QString("%1W").arg(m_propUsePower - m_batUsePower, 0, 'f', 1));
	}
	else
	{
		m_cellToPropPath->setVisible(false);
		m_cellUsePowerText->setVisible(false);
		m_cellPowerText->setPlainText(QString("0W"));
	}
}

//void EnergyBudget::resizeEvent(QResizeEvent *event)
//{
//    QWidget::resizeEvent(event);
//    ui->overviewGraphicsView->fitInView(m_scene->sceneRect(), Qt::AspectRatioMode::KeepAspectRatio);
//}

void EnergyBudget::setActiveUAS(Vehicle* vehicle)
{
    //disconnect any previous uas
    disconnect(this, SLOT(updatePower(float, float, float, float)));
    disconnect(this, SLOT(updatePowerHL(uint8_t)));
    disconnect(this, SLOT(updateMPPT(float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t)));
    disconnect(this, SLOT(updateMPPTHL(uint8_t, uint8_t, uint8_t)));
    disconnect(this, SLOT(updateBatMon(uint8_t, uint16_t, int16_t, uint8_t, float, uint16_t, uint16_t, uint16_t,uint8_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t)));
    disconnect(this, SLOT(updateBatMonHL(uint8_t, uint8_t, int8_t, uint8_t)));
    disconnect(this, SLOT(onThrustChanged(Vehicle*,double)));
    disconnect(this, SLOT(onSensPowerBoardChanged(uint8_t)));

    //connect the uas if asluas
    Vehicle* tempUAS = vehicle;

    connect(tempUAS, SIGNAL(SensPowerChanged(float, float, float, float)), this, SLOT(updatePower(float, float, float, float)));
    connect(tempUAS, SIGNAL(SensPowerChangedHL(uint8_t)), this, SLOT(updatePowerHL(uint8_t)));
    connect(tempUAS, SIGNAL(MPPTDataChanged(float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t)), this, SLOT(updateMPPT(float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t, float, float, uint16_t, uint8_t)));
    connect(tempUAS, SIGNAL(MPPTDataChangedHL(uint8_t, uint8_t, uint8_t)), this, SLOT(updateMPPTHL(uint8_t, uint8_t, uint8_t)));
    connect(tempUAS, SIGNAL(BatMonDataChanged(uint8_t, uint16_t, int16_t, uint8_t, float, uint16_t, uint32_t, uint32_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t)), this, SLOT(updateBatMon(uint8_t, uint16_t, int16_t, uint8_t, float, uint16_t, uint32_t, uint32_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t)));
    connect(tempUAS, SIGNAL(BatMonDataChangedHL(uint8_t, uint8_t, int8_t, uint8_t)), this, SLOT(updateBatMonHL(uint8_t, uint8_t, int8_t, uint8_t)));

    connect(tempUAS, SIGNAL(thrustChanged(Vehicle*, double)), this, SLOT(onThrustChanged(Vehicle*, double)));
    connect(tempUAS, SIGNAL(SensPowerBoardChanged(uint8_t)), this, SLOT(onSensPowerBoardChanged(uint8_t)));
}

#define HOSTFETBIT1 "DSG"
#define HOSTFETBIT2 "CHG"
#define HOSTFETBIT3 "PCHG"

QString EnergyBudget::convertHostfet(uint16_t bit)
{
	if (bit & 0x0002) return HOSTFETBIT1;
	if (bit & 0x0004) return HOSTFETBIT2;
	if (bit & 0x0008) return HOSTFETBIT3;
	return "OK";
}

#define BATTERYBit15 "OCA"
#define BATTERYBit14 "TCA"
#define BATTERYBit12 "OTA"
#define BATTERYBit11 "TDA"
#define BATTERYBit9 "RCA"
#define BATTERYBit8 "RTA"
#define BATTERYBit7 "INIT"
#define BATTERYBit6 "DSG"
#define BATTERYBit5 "FC"
#define BATTERYBit4 "FD"

QString EnergyBudget::convertBatteryStatus(uint16_t bit)
{
	if (bit & 0x0010) return BATTERYBit4;
	if (bit & 0x0020) return BATTERYBit5;
	if (bit & 0x0040) return BATTERYBit6;
	if (bit & 0x0080) return BATTERYBit7;
	if (bit & 0x0100) return BATTERYBit8;
	if (bit & 0x0200) return BATTERYBit9;
	if (bit & 0x0800) return BATTERYBit11;
	if (bit & 0x1000) return BATTERYBit12;
	if (bit & 0x4000) return BATTERYBit14;
	if (bit & 0x8000) return BATTERYBit15;
	return "OK";
}

#define MPPTBit0Active "CV"
#define MPPTBit0Inactive "CC"
#define MPPTBit1 "OVA"
#define MPPTBit2 "OTA"
#define MPPTBit3 "OCAL4"
#define MPPTBit4 "OCAL3"
#define MPPTBit5 "OCAL2"
#define MPPTBit6 "OCAL1"

QString EnergyBudget::convertMPPTModus(uint8_t bit)
{
	if (bit & 0x0001) return MPPTBit0Active;
	return MPPTBit0Inactive;
}

QString EnergyBudget::convertMPPTStatus(uint8_t bit)
{
	if (bit & 0x0002) return MPPTBit1;
	if (bit & 0x0004) return MPPTBit2;
	if (bit & 0x0008) return MPPTBit3;
	if (bit & 0x0010) return MPPTBit4;
	if (bit & 0x0020) return MPPTBit5;
	if (bit & 0x0040) return MPPTBit6;
	return "OK";
}

void EnergyBudget::ResetMPPTCmd(void)
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, tr("MPPT reset"), tr("Sending command to reset MPPT. Use this with caution! Are you sure?"), QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		int MPPTNr = ui->ResetMPPTEdit->text().toInt();

        //Send the message via the currently active UAS
        Vehicle* tempUAS = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle();
        if (tempUAS) {

            mavlink_message_t       msg;
            mavlink_command_long_t  cmd;

            cmd.command = MAV_CMD_RESET_MPPT;
            cmd.confirmation = 0;

            cmd.param1 = (float) MPPTNr;
            cmd.param2 = 0.0f;
            cmd.param3 = 0.0f;
            cmd.param4 = 0.0f;
            cmd.param5 = 0.0f;
            cmd.param6 = 0.0f;
            cmd.param7 = 0.0f;

            cmd.target_system = tempUAS->id();
            cmd.target_component = tempUAS->defaultComponentId();
            mavlink_msg_command_long_encode_chan(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                                 qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                                 tempUAS->priorityLink()->mavlinkChannel(),
                                                 &msg,
                                                 &cmd);

            tempUAS->sendMessageOnLink(tempUAS->priorityLink(), msg);
        }
	}
}

void EnergyBudget::styleChanged(bool darkStyle)
{
	if (darkStyle)
	{
		m_chargePowerText->setDefaultTextColor(QColor(Qt::white));
		m_batUsePowerText->setDefaultTextColor(QColor(Qt::white));
		m_cellPowerText->setDefaultTextColor(QColor(Qt::white));
		m_cellUsePowerText->setDefaultTextColor(QColor(Qt::white));
		m_SystemUsePowerText->setDefaultTextColor(QColor(Qt::white));
	}
	else
	{
		m_chargePowerText->setDefaultTextColor(QColor(Qt::black));
		m_batUsePowerText->setDefaultTextColor(QColor(Qt::black));
		m_cellPowerText->setDefaultTextColor(QColor(Qt::black));
		m_cellUsePowerText->setDefaultTextColor(QColor(Qt::black));
		m_SystemUsePowerText->setDefaultTextColor(QColor(Qt::black));
	}
}

void EnergyBudget::MPPTTimerTimeout(void)
{
	m_cellPower = m_propUsePower - m_chargePower - m_batUsePower;
	ui->mppt1VLabel->setText(QString("--"));
	ui->mppt2VLabel->setText(QString("--"));
	ui->mppt3VLabel->setText(QString("--"));
	ui->mppt1ALabel->setText(QString("--"));
	ui->mppt2ALabel->setText(QString("--"));
	ui->mppt3ALabel->setText(QString("--"));
}

void EnergyBudget::onThrustChanged(Vehicle* vehicle, double thrust)
{
    Q_UNUSED(vehicle);
	m_thrust = thrust;
}

Hysteresisf::Hysteresisf(float minVal, float maxVal, bool isHighState) :
m_minVal(minVal),
m_maxVal(maxVal),
m_highState(isHighState)
{

}

bool Hysteresisf::check(float const currVal)
{
	if (currVal < m_minVal)
	{
		m_highState = false;
	}
	if (currVal > m_maxVal)
	{
		m_highState = true;
	}
	return m_highState;
}
