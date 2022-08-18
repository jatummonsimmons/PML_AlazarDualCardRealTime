#include "dataProcessingThread.h"

dataProcessingThread::dataProcessingThread(QObject *parent) :
    QThread(parent),
    rawSig_ch1(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch2(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch3(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch4(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_flag(false),
    avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    avgSig_flag(false),
    sig_m1(RECORDS_PER_BUFFER),
    sig_m2(RECORDS_PER_BUFFER),
    sig_sc(RECORDS_PER_BUFFER),
    sig_pa(RECORDS_PER_BUFFER),
    sig_gsc(RECORDS_PER_BUFFER),
    sig_flag(false),
    focusSig_flag(false)
{

}


dataProcessingThread::~dataProcessingThread()
{
    qDebug() << "PTh: Desctructor called...";
    wait();
    qDebug() << "PTh: Desctructor finished...";
}

void dataProcessingThread::run()
{
    exec();
}


void dataProcessingThread::read_rawSig(QVector<double> *ch1,
                                       QVector<double> *ch2,
                                       QVector<double> *ch3,
                                       QVector<double> *ch4)
{
    QMutexLocker locker(&rawSig_mutex);

    *ch1 = rawSig_ch1[0];
    *ch2 = rawSig_ch2[0];
    *ch3 = rawSig_ch3[0];
    *ch4 = rawSig_ch4[0];

    rawSig_flag = false;
}

void dataProcessingThread::read_avgSig(QVector< QVector<double> > *avgSig_ch1)
{
    QMutexLocker locker(&avgSig_mutex);

    *avgSig_ch1 = this->avgSig;

    avgSig_flag = false;
}

void dataProcessingThread::read_sig(QVector<double> *sig_pa,
                                    QVector<double> *sig_sc,
                                    QVector<double> *sig_gsc,
                                    QVector<double> *sig_m1,
                                    QVector<double> *sig_m2)
{
    QMutexLocker locker(&sig_mutex);
    *sig_pa = this->sig_pa;
    *sig_sc = this->sig_sc;
    *sig_gsc = this->sig_gsc;
    *sig_m1 = this->sig_m1;
    *sig_m2 = this->sig_m2;

    sig_flag = false;
}


void dataProcessingThread::read_focusSig(double *focusSig_nsc){
    QMutexLocker locker(&focusSig_mutex);
    *focusSig_nsc = this->focusSig_nsc;
    focusSig_flag = false;
}

void dataProcessingThread::updateTimeDomains(AlazarControlThread *dataThread)
{
    // Raw signal emission of one data point
    QVector< QVector<double> > temp_rawSig_ch1(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch2(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch3(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch4(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    bool rawSig_sendEmit;

    dataThread->readLatestData(&temp_rawSig_ch1,
                               &temp_rawSig_ch2,
                               &temp_rawSig_ch3,
                               &temp_rawSig_ch4);
/* Commented out to simplify UI
    {
        QMutexLocker locker(&rawSig_mutex);

        rawSig_ch1 = temp_rawSig_ch1;
        rawSig_ch2 = temp_rawSig_ch2;
        rawSig_ch3 = temp_rawSig_ch3;
        rawSig_ch4 = temp_rawSig_ch4;

        rawSig_sendEmit = !rawSig_flag;
        rawSig_flag = true;
    }

    if (rawSig_sendEmit)
    {
        emit rawSig_ready();
    }
*/
    // Average signal emission
    QVector< QVector<double> > temp_avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_alignedSig(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));

    bool avgSig_sendEmit;

    // Align the time domains in height
    for (int i =0; i < RECORDS_PER_BUFFER; i++){
        double DC = 0;
        for (int j = 0; j < PRE_TRIGGER_SAMPLES-50; j++){
            DC += temp_rawSig_ch1[i][j];
        }
        DC /= (PRE_TRIGGER_SAMPLES-50);
        for (int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            temp_alignedSig[i][j] = temp_rawSig_ch1[i][j] - DC;
        }
    }
    // Average the time domains along the record axis
    for(int i = 0; i < NUM_AVERAGE_SIGNALS;i++){
        for(int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            for (int k = 0; k < RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS; k++){
                temp_avgSig[i][j] += temp_alignedSig[k+i*RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS][j];
            }
            temp_avgSig[i][j] /= RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS;
        }
    }

    {
        QMutexLocker locker(&avgSig_mutex);
        avgSig = temp_avgSig;
        avgSig_sendEmit = !avgSig_flag;
        avgSig_flag = true;
    }

    if (avgSig_sendEmit)
    {
        emit avgSig_ready();
    }

    // Extracted signal emission for all data points
    QVector< double > temp_sig_m1(RECORDS_PER_BUFFER);
    QVector< double > temp_sig_m2(RECORDS_PER_BUFFER);
    QVector< double > temp_sig_pa(RECORDS_PER_BUFFER);
    QVector< double > temp_sig_sc(RECORDS_PER_BUFFER);
    QVector< double > temp_sig_gsc(RECORDS_PER_BUFFER);
    bool sig_sendEmit;

    // MIRROR 1 EXTRACTION
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        for (int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            temp_sig_m1[i] += temp_rawSig_ch3[i][j];
        }
        temp_sig_m1[i] /= (PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
        temp_sig_m1[i] += -0.0028;

    }

    // MIRROR 2 EXTRACTION
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        for (int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            temp_sig_m2[i] += temp_rawSig_ch4[i][j];
        }
        temp_sig_m2[i] /= (PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
        temp_sig_m2[i] += -0.002;
    }
    // NIR SCATTERING EXTRACTION
    //Only averaging pre-trigger because it avoids any corruption from PARS
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        for (int j = 0; j < 80; j++){
            temp_sig_sc[i] += temp_rawSig_ch1[i][j];
        }
        temp_sig_sc[i] /= (80);
    }

    // PARS SIGNAL EXTRACTION
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        // Iterate over pre trigger region to extract DC
        double DC = 0;
        for (int j = 0; j < PRE_TRIGGER_SAMPLES-50; j++){
            DC += temp_rawSig_ch1[i][j];
        }
        DC /= (PRE_TRIGGER_SAMPLES-50);

        for (int j = 90;j < 130;j++){
            temp_sig_pa[i] += temp_rawSig_ch1[i][j];
        }
        temp_sig_pa[i] /= (40);
        temp_sig_pa[i] -= DC;
        // If desired can scattering compensate
        //temp_sig_pa[i] /= DC;

    }

    // GREEN SCATTERING EXTRACTION
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        double gsc_DC = 0;
        for (int j = 0; j < 80; j++){
            gsc_DC += temp_rawSig_ch2[i][j];
        }
        gsc_DC /= 80;
        for (int j = 90; j < 384; j++){
            temp_sig_gsc[i] += temp_rawSig_ch2[i][j];
        }
        temp_sig_gsc[i] /= 274;
        temp_sig_gsc[i] -= gsc_DC;
    }

    {
        QMutexLocker locker(&sig_mutex);
        sig_pa = temp_sig_pa;
        sig_sc = temp_sig_sc;
        sig_gsc = temp_sig_gsc;
        sig_m1 = temp_sig_m1;
        sig_m2 = temp_sig_m2;
        sig_sendEmit = !sig_flag;
        sig_flag = true;
    }

    if (sig_sendEmit){
        emit sig_ready();
    }

    // NIR SCATTERING FOCUS METRIC
    double temp_focusSig_nsc = 0;
    bool focusSig_sendEmit;

    for (int i = 0; i < RECORDS_PER_BUFFER-1; i++){
        temp_focusSig_nsc += temp_sig_sc[i]-temp_sig_sc[i+1];
    }
    temp_focusSig_nsc = temp_focusSig_nsc/(RECORDS_PER_BUFFER-1);

    {
        QMutexLocker locker(&focusSig_mutex);
        focusSig_nsc = temp_focusSig_nsc;
        focusSig_sendEmit = !focusSig_flag;
        focusSig_flag = true;
    }

    if (focusSig_sendEmit){
        emit focusSig_ready();
    }
}
