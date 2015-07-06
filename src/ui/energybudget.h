#ifndef ENERGYBUDGET_H
#define ENERGYBUDGET_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
class EnergyBudget;
}

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsPathItem;
class QGraphicsTextItem;
class UASInterface;

class EnergyBudget : public QWidget
{
    Q_OBJECT

public:
    explicit EnergyBudget(QWidget *parent = 0);
    ~EnergyBudget();

protected:
    Ui::EnergyBudget *ui;
	QGraphicsScene *m_scene;
	QGraphicsPixmapItem *m_propPixmap;
	QGraphicsPixmapItem *m_cellPixmap;
	QGraphicsPixmapItem *m_batPixmap;
	QGraphicsPathItem *m_chargePath;
	QGraphicsPathItem *m_cellToPropPath;
	QGraphicsPathItem *m_batToPropPath;
	QGraphicsTextItem *m_chargePowerText;
	QGraphicsTextItem *m_cellPowerText;
	QGraphicsTextItem *m_cellUsePowerText;
	QGraphicsTextItem *m_batUsePowerText;
	QGraphicsTextItem *m_SystemUsePowerText;
	float m_cellPower;
	float m_batUsePower;
	float m_propUsePower;
	float m_chargePower;
	bool m_batCharging;

	void buildGraphicsImage();
	qreal adjustImageScale(const QRectF &, QRectF&);
	virtual void resizeEvent(QResizeEvent * event);
	void updateGraphicsImage();
	QString convertHostfet(uint16_t);
	QString convertBatteryStatus(uint16_t);
	QString convertMPPTStatus(uint8_t bit);

protected slots:
	void updatePower(float volt, float currpb, float curr_1, float curr_2);
	void updateMPPT(float volt1, float amp1, uint16_t pwm1, uint8_t status1, float volt2, float amp2, uint16_t pwm2, uint8_t status2, float volt3, float amp3, uint16_t pwm3, uint8_t status3);
	void updateBatMon(uint8_t compid, uint16_t volt, int16_t current, uint8_t soc, float temp, uint16_t batStatus, uint16_t hostfetcontrol, uint16_t cellvolt1, uint16_t cellvolt2, uint16_t cellvolt3, uint16_t cellvolt4, uint16_t cellvolt5, uint16_t cellvolt6);
	void setActiveUAS(UASInterface *uas);
	void styleChanged(bool);

	void ResetMPPTCmd();
};

#endif // ENERGYBUDGET_H
