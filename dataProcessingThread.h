#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <AlazarControlThread.h>
#include <acquisitionConfig.h>

class dataProcessingThread : public QThread
{
    Q_OBJECT

public:
    dataProcessingThread(QObject *parent = nullptr);
    ~dataProcessingThread();
    void read_rawSig(QVector<double> *ch1,
                     QVector<double> *ch2,
                     QVector<double> *ch3,
                     QVector<double> *ch4);
    void read_avgSig(QVector<QVector<double>> *avgSig_ch1);
    void read_sig(QVector<double> *sig_pa,
                  QVector<double> *sig_sc,
                  QVector<double> *sig_gsc,
                  QVector<double> *sig_m1,
                  QVector<double> *sig_m2);

signals:
    void rawSig_ready();
    void avgSig_ready();
    void sig_ready();

protected:
    void run() override;

public slots:
    void updateTimeDomains(AlazarControlThread *dataThread);

private:
    QMutex rawSig_mutex;
    QVector< QVector<double> > rawSig_ch1;
    QVector< QVector<double> > rawSig_ch2;
    QVector< QVector<double> > rawSig_ch3;
    QVector< QVector<double> > rawSig_ch4;
    bool rawSig_flag;

    QMutex avgSig_mutex;
    QVector< QVector<double> > avgSig;
    bool avgSig_flag;

    QMutex sig_mutex;
    QVector< double > sig_m1;
    QVector< double > sig_m2;
    QVector< double > sig_sc;
    QVector< double > sig_pa;
    QVector< double > sig_gsc;
    bool sig_flag;

};

#endif // DATAPROCESSINGTHREAD_H
