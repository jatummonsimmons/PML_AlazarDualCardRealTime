#ifndef ALAZARCONTROLTHREAD_H
#define ALAZARCONTROLTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QTimer>
#include <QDebug>

#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <stdlib.h>
#include <atomic>
#include <chrono>

#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

#include "acquisitionConfig.h"

#ifdef _WIN32
#include <conio.h>
#else // ifndef _WIN32
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define TRUE  1
#define FALSE 0

#define _snprintf snprintf

inline U32 GetTickCount(void);
inline void Sleep(U32 dwTime_ms);
inline int _kbhit (void);
inline int GetLastError();
#endif // ifndef _WIN32

#define BUFFER_COUNT 8

class AlazarControlThread : public QThread
{
    Q_OBJECT

public:
    AlazarControlThread(QObject *parent = nullptr);
    ~AlazarControlThread();
    void readLatestData(QVector< QVector<double> > *ch1,
                        QVector< QVector<double> > *ch2,
                        QVector< QVector<double> > *ch3,
                        QVector< QVector<double> > *ch4);
    void stopRunning();
    int saveDataBuffer();
    void startContinuousSave(int numBuffer);
    void stopContinuousSave();

signals:
    void dataReady(AlazarControlThread * dataThread);
    void continuousSaveComplete();

protected:
    void run() override;

private:
    bool ConfigureBoard(HANDLE boardHandle);
    bool AcquireData(HANDLE boardHandle);
    double InputRangeIdToVolts(U32 inputRangeId);

    U16 *BufferArray[BUFFER_COUNT] = { NULL };
    double samplesPerSec = 0.0;
    U32 inputRange[4];

    U32 preTriggerSamples;
    U32 postTriggerSamples;
    U32 recordsPerBuffer;
    U32 buffersPerAcquisition;

    U32 bytesPerBuffer;
    std::atomic<U32> numSaveBufferAtom;
    std::atomic<bool> flagAtom;

    U16 * saveBuffer;
    double * waitTimeBuffer;

    QMutex mutex;
    bool flag;
    bool running;
    bool pauseSaveBuffer;

    // Declarations for continuous save operations including startContinuousSave() and stopContinuousSave()
    bool saveData;
    U32 currentSaveCount;
    U32 totalSaveCount;
    FILE *continuousSaveFile;

};

#endif // ALAZARCONTROLTHREAD_H
