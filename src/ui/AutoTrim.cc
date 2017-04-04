#include "AutoTrim.h"
#include <QMessageBox>
#include "QGCApplication.h"
#include "MultiVehicleManager.h"


#include "ui_AutoTrim.h"
#include <QList>
#include <QDebug>

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

const float RAD2DEG = 180.0f/3.14159f; //assumes multiplication with this constant for conversion

//*******************************************************
//*** Initialization (Construction/Destruction)
//*******************************************************

AutoTrim::AutoTrim(const QString& title, QAction* action, QWidget *parent) :
    QGCDockWidget(title, action, parent)
  , lastVoltageWarning(0)
  , m_ui(new Ui::AutoTrim)
  , _lpVoltage_ext(0.0)
  , _tickLowpassVoltage_ext(0.0)
  , _lastTickVoltageValue_ext(0.0)
  , _emptyVoltage_ext(9.0)
  , _warnVoltage_ext(10.5)
  , _fullVoltage_ext(12.5)
  , _tickVoltage_ext(_warnVoltage_ext)
  , _startVoltage_ext(-1.0f)
{
    m_ui->setupUi(this);

    // Connect buttons
    connect(m_ui->pb_Connect, SIGNAL(clicked()), this, SLOT(ConnectToActiveUAS()));
    connect(m_ui->pb_StartTrim, SIGNAL(clicked()), this, SLOT(StartTrim()));
    connect(m_ui->pb_StopTrim, SIGNAL(clicked()), this, SLOT(StopTrim()));
    //connect(m_ui->pb_SetVMin, SIGNAL(clicked()), this, SLOT(SetTrimDataVMin()));
    //connect(m_ui->pb_SetVNom, SIGNAL(clicked()), this, SLOT(SetTrimDataVNom()));
    //connect(m_ui->pb_SetVMax, SIGNAL(clicked()), this, SLOT(SetTrimDataVMax()));

    m_ui->pb_Connect->setEnabled(true);
    m_ui->pb_StartTrim->setEnabled(false);
    m_ui->pb_StopTrim->setEnabled(false);

    // Connect slots to externals signals later on click on the Connect()-Button

    //Reset all data
    ResetData();
    bStarted = false;
    bConnected = false;
}

AutoTrim::~AutoTrim()
{
}

//*******************************************************
//*** Internal Slots
//*******************************************************

void AutoTrim::ConnectToActiveUAS(void)
{
    // Get pointer to UAS first
    //Send the message via the currently active UAS

    Vehicle* tempUAS = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle();    

    // Start connection here
    int ret = QMessageBox::information(this, tr("[AutoTrim] Connect"), tr("Connecting to UAS...Please always make sure that you have dynamic-pressure scaling disabled on the autopilot before starting the auto-trim!"), QMessageBox::Ok | QMessageBox::Cancel);
    if (ret != QMessageBox::Ok) {
            return;
    }

    if(tempUAS != NULL) {
        connect(tempUAS, SIGNAL(AslctrlDataChanged(float,float,float,float,float,float,float,float,float,float)), this, SLOT(OnAslctrlDataChanged(float,float,float,float,float,float,float,float,float,float)));
        connect(tempUAS, SIGNAL(speedChanged(Vehicle*, double, double)), this, SLOT(OnSpeedChanged(Vehicle*, double, double)));
        connect(tempUAS, SIGNAL(SensPowerChanged(float,float,float,float)), this, SLOT(OnSensPowerChanged(float,float,float,float)));
        QMessageBox::information(this, tr("[AutoTrim] Connect"),tr("Connected to the active UAS!"), QMessageBox::Ok);
        m_ui->pb_Connect->setEnabled(false);
        m_ui->pb_StartTrim->setEnabled(true);
        bConnected = true;
    }
    else QMessageBox::information(this, tr("[AutoTrim] Connect"), tr("Connect failed! Make sure there is an active UAS and repeat!"), QMessageBox::Ok);
}

void AutoTrim::StartTrim(void)
{
    //Reset / Restart
    ResetData();
    bStarted = true;
    m_ui->pb_StartTrim->setEnabled(false);
    m_ui->pb_StopTrim->setEnabled(true);
}

void AutoTrim::StopTrim(void)
{
    // Finalize display of average/accumulated values here. Do NOT set the values here.
    bStarted = false;
    m_ui->pb_StartTrim->setEnabled(true);
    m_ui->pb_StopTrim->setEnabled(false);
}

void AutoTrim::SetTrimDataVMin(void)
{
    SetTrimData(1);
}
void AutoTrim::SetTrimDataVNom(void)
{
    SetTrimData(2);
}
void AutoTrim::SetTrimDataVMax(void)
{
    SetTrimData(3);
}
void AutoTrim::SetTrimData(int velocityType)
{
    QMessageBox::information(this, tr("[AutoTrim] StartTrim"),tr("Not implemented. Please set the shown average trim values manually. "), QMessageBox::Ok);
    Q_UNUSED(velocityType);
    return;

    /*if(bStarted) {
            QMessageBox::information(this, tr("[AutoTrim] StartTrim"),tr("Please stop the data recording before setting the parameters. Aborting. "), QMessageBox::Ok);
            return;
    }*/

    //Get handle to the parameter item

    //parameterInterface::paramWidgets; //(protected)

    // Do this for all params which have to be changed.
    //QGCParamWidget::handleOnboardParamUpdate(int component,const QString& parameterName, QVariant value);

    //

    // -> Check the UAS Parameter manager! This seems nice!
}

//*******************************************************
//*** External Slots
//*******************************************************

void AutoTrim::OnAslctrlDataChanged(float uElev, float uAil, float uRud, float uThrot, float roll, float pitch, float yaw, float roll_ref, float pitch_ref, float h)
{
    if (bConnected){
            // Update current data text fields.
            m_ui->e_cur_uElev->setText(QString("%1").arg(uElev, 0, 'f', 4));
            m_ui->e_cur_uAil->setText(QString("%1").arg(uAil, 0, 'f', 4));
            m_ui->e_cur_uRud->setText(QString("%1").arg(uRud, 0, 'f', 4));
            m_ui->e_cur_uThrot->setText(QString("%1").arg(uThrot, 0, 'f', 4));
            m_ui->e_cur_roll->setText(QString("%1").arg(roll, 0, 'f', 2));
            m_ui->e_cur_pitch->setText(QString("%1").arg(pitch, 0, 'f', 2));
            m_ui->e_ref_roll->setText(QString("%1").arg(roll_ref, 0, 'f', 2));
            m_ui->e_ref_pitch->setText(QString("%1").arg(pitch_ref, 0, 'f', 2));
            m_ui->e_cur_h->setText(QString("%1").arg(h, 0, 'f', 2));
            m_ui->e_cur_yaw->setText(QString("%1").arg(yaw, 0, 'f', 2));
    }

    if (bStarted) {
        // Update avrg/accum values here.
        avg_uElev = (avg_uElev*n_u + uElev) / (n_u + 1);
        avg_uAil = (avg_uAil*n_u + uAil) / (n_u + 1);
        avg_uRud = (avg_uRud*n_u + uRud) / (n_u + 1);
        avg_uThrot = (avg_uThrot*n_u + uThrot) / (n_u + 1);
        avg_roll = (avg_roll*n_u + roll) / (n_att + 1);
        avg_pitch = (avg_pitch*n_u + pitch) / (n_att + 1);

        if (n_h == 0) h_Start = h;
        if (n_att == 0) yaw_Start = yaw;

        // Update accumulated & average data fields
        m_ui->e_avg_uElev->setText(QString("%1").arg(avg_uElev, 0, 'f', 4));
        m_ui->e_avg_uAil->setText(QString("%1").arg(avg_uAil, 0, 'f', 4));
        m_ui->e_avg_uRud->setText(QString("%1").arg(avg_uRud, 0, 'f', 4));
        m_ui->e_avg_uThrot->setText(QString("%1").arg(avg_uThrot, 0, 'f', 4));
        m_ui->e_avg_roll->setText(QString("%1").arg(avg_roll, 0, 'f', 2));
        m_ui->e_avg_pitch->setText(QString("%1").arg(avg_pitch, 0, 'f', 2));
        m_ui->e_delta_h->setText(QString("%1").arg(h-h_Start, 0, 'f', 2));
        m_ui->e_delta_yaw->setText(QString("%1").arg(yaw-yaw_Start, 0, 'f', 2));

        // Increase counters
        n_u += 1;
        n_att += 1;
        n_h += 1;

        //Update elapsed time and counters
        UpdateElapsedTimeCounters();
    }
}

void AutoTrim::OnSpeedChanged(Vehicle* vehicle, double groundspeed, double airspeed)
{
    Q_UNUSED(vehicle);
    Q_UNUSED(groundspeed);

    if (bConnected) {
            // Update current data text fields.
            m_ui->e_cur_airspeed->setText(QString("%1").arg(airspeed, 0, 'f', 2));
    }

    if (bStarted) {
            // Update avrg/accum values here.
            avg_airspeed = (avg_airspeed*n_airspeed + (float)airspeed) / (n_airspeed + 1);
            m_ui->e_avg_airspeed->setText(QString("%1").arg(avg_airspeed, 0, 'f', 2));

            // Increase counters
            n_airspeed += 1;

            //Update elapsed time and counters
            UpdateElapsedTimeCounters();
    }
}

void AutoTrim::OnSensPowerChanged(float volt, float currpb, float curr_1, float curr_2)
{
    Q_UNUSED(curr_1);
    Q_UNUSED(curr_2);

    // Battery charge/time remaining/voltage calculations
    _lpVoltage_ext = volt;
    _tickLowpassVoltage_ext = _tickLowpassVoltage_ext*0.8f + 0.2f*_lpVoltage_ext;
    // We don't want to tick above the threshold
    if (_tickLowpassVoltage_ext > _tickVoltage_ext)
    {
        _lastTickVoltageValue_ext = _tickLowpassVoltage_ext;
    }
    if ((_startVoltage_ext > 0.0f) && (_tickLowpassVoltage_ext < _tickVoltage_ext) && (fabs(_lastTickVoltageValue_ext - _tickLowpassVoltage_ext) > 0.1f)
        /* warn if lower than treshold */
        && (_lpVoltage_ext < _tickVoltage_ext)
        /* warn only if we have at least the voltage of an empty LiPo cell, else we're sampling something wrong */
        && (_lpVoltage_ext > 3.3f)
        /* warn only if current voltage is really still lower by a reasonable amount */
        && ((_lpVoltage_ext - 0.2f) < _tickVoltage_ext)
        /* warn only every 12 seconds */
        && (QGC::groundTimeUsecs() - lastVoltageWarning) > 12000000)
    {
        qgcApp()->toolbox()->audioOutput()->say(QString("ADC121 Voltage warning for system %1: %2 volts").arg(qgcApp()->toolbox()->multiVehicleManager()->activeVehicle()->id()).arg(_lpVoltage_ext, 0, 'f', 1, QChar(' ')));
        lastVoltageWarning = QGC::groundTimeUsecs();
        _lastTickVoltageValue_ext = _tickLowpassVoltage_ext;
    }

    if (_startVoltage_ext == -1.0f && _lpVoltage_ext > 0.1f) _startVoltage_ext = _lpVoltage_ext;

    if (bConnected) {
            // Update current data text fields.
            m_ui->e_cur_power->setText(QString("%1").arg(_lpVoltage_ext*currpb, 0, 'f', 2));
    }

    if (bStarted) {
            // Update avrg/accum values here.
            avg_power = (avg_power*n_power + _lpVoltage_ext*currpb) / (n_power + 1);
            m_ui->e_avg_power->setText(QString("%1").arg(avg_power, 0, 'f', 2));

            // Increase counters
            n_power += 1;

            //Update elapsed time and counters
            UpdateElapsedTimeCounters();
    }

}

void AutoTrim::ResetData()
{
    tElapsedSinceStart.restart();

    n_u=0;
    avg_uElev=0.0f;
    avg_uAil=0.0f;
    avg_uRud=0.0f;
    avg_uThrot=0.0f;

    n_att=0;
    avg_roll=0.0f;
    avg_pitch=0.0f;
    yaw_Start=0.0f;

    n_airspeed=0;
    avg_airspeed=0.0f;

    n_power=0;
    avg_power=0.0f;

    n_h=0;
    h_Start=0.0f;
}

void AutoTrim::UpdateElapsedTimeCounters(void)
{
    float time_seconds=((float)tElapsedSinceStart.elapsed()/1000.0f);
    m_ui->e_timeelapsed->setText(QString("%1s").arg(time_seconds, 0, 'f', 2));
    QString temp=QString("%1;%2").arg(n_u).arg(n_airspeed);
    m_ui->e_counters->setText(temp);
}


