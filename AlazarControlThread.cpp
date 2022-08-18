#include "AlazarControlThread.h"

AlazarControlThread :: AlazarControlThread(QObject *parent) :
    QThread(parent),
    flag(false),
    running(true),
    pauseSaveBuffer(false)
{
    numSaveBufferAtom = 0;
    flagAtom = false;
}


AlazarControlThread :: ~AlazarControlThread()
{
    qDebug() << "DTh: Destructor called...";
    stopRunning();
    wait();
    delete [] saveBuffer;
    qDebug() << "DTh: Destructor finished...";
}

void AlazarControlThread::run()
{
    //direct copy paste of NPT
    U32 systemId = 1;
    U32 boardId = 1;

    // Get a handle to the board

//    HANDLE boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
//    ALAZAR_BOARDTYPES boardType = AlazarGetBoardKind(boardHandle);
//    if (boardType ==18){
//        U32 systemId = 2;
//        HANDLE boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
//    }
//    if (boardHandle == NULL)
//    {
//        printf("Error: Unable to open board system Id %u board Id %u\n", systemId, boardId);
//        return;
//    }

    // Get a handle to each board in the board system
    HANDLE boardHandleArray[2] = { NULL };
    U32 boardIndex;
    for (boardIndex = 0; boardIndex < 2; boardIndex++){
        U32 boardId = boardIndex + 1;
        boardHandleArray[boardIndex] = AlazarGetBoardBySystemID(systemId, boardId);
        if (boardHandleArray[boardIndex] == NULL){
            printf("Error in AlazarGetBoardBySystemID(): Unable to open board system Id %u board Id %u\n", systemId, boardId);
            return;
        }
    }


    // Configure the board's sample rate, input, and trigger settings

    for (boardIndex = 0; boardIndex < 2; boardIndex++){
        if (!ConfigureBoard(boardHandleArray[boardIndex]))
        {
            qDebug() << "Error: Configure board failed for board number:" << boardIndex+1 << "with handle" << boardHandleArray[boardIndex];
            return;
        }
    }
    // Make an acquisition, optionally saving sample data to a file

    // Passing just the first of the two boards which acts as the systemHandle
    if (!AcquireData(boardHandleArray))
    {
        printf("Error: Acquisition failed\n");
        return;
    }

    return;
}

bool AlazarControlThread::ConfigureBoard(HANDLE boardHandle)
{
    RETURN_CODE retCode;

    // TODO: Specify the sample rate (see sample rate id below)

    samplesPerSec = 125000000.0;

    // TODO: Select clock parameters as required to generate this sample rate.
    //
    // For example: if samplesPerSec is 100.e6 (100 MS/s), then:
    // - select clock source INTERNAL_CLOCK and sample rate SAMPLE_RATE_100MSPS
    // - select clock source FAST_EXTERNAL_CLOCK, sample rate SAMPLE_RATE_USER_DEF, and connect a
    //   100 MHz signal to the EXT CLK BNC connector.

    retCode = AlazarSetCaptureClock(boardHandle,
                                    INTERNAL_CLOCK,
                                    SAMPLE_RATE_250MSPS,
                                    CLOCK_EDGE_RISING,
                                    0);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarSetCaptureClock failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }


    // Channel settings
    // Select channel A input parameters as required
    retCode = AlazarInputControlEx(boardHandle, CHANNEL_A, DC_COUPLING, INPUT_RANGE_PM_1_V_25, IMPEDANCE_50_OHM);
    if (retCode != ApiSuccess){
        printf("Error: AlazarInputControlEx failed -- %s\n", AlazarErrorToText(retCode));return FALSE;
    }

    // Select channel A bandwidth limit as required
    retCode = AlazarSetBWLimit(boardHandle, CHANNEL_A, 0);
    if (retCode != ApiSuccess){
        printf("Error: AlazarSetBWLimit failed -- %s\n", AlazarErrorToText(retCode)); return FALSE;
    }

    // Select channel B input parameters as required
    retCode = AlazarInputControlEx(boardHandle, CHANNEL_B, DC_COUPLING, INPUT_RANGE_PM_1_V_25, IMPEDANCE_50_OHM);
    if (retCode != ApiSuccess){
        printf("Error: AlazarInputControlEx failed -- %s\n", AlazarErrorToText(retCode)); return FALSE;
    }

    // Select channel B bandwidth limit as required
    retCode = AlazarSetBWLimit(boardHandle, CHANNEL_B, 0);
    if (retCode != ApiSuccess){
        printf("Error: AlazarSetBWLimit failed -- %s\n", AlazarErrorToText(retCode)); return FALSE;
    }

    // TODO: Select trigger inputs and levels as required

    retCode = AlazarSetTriggerOperation(boardHandle,
                                        TRIG_ENGINE_OP_J,
                                        TRIG_ENGINE_J,
                                        TRIG_EXTERNAL,
                                        TRIGGER_SLOPE_NEGATIVE,
                                        150,
                                        TRIG_ENGINE_K,
                                        TRIG_DISABLE,
                                        TRIGGER_SLOPE_POSITIVE,
                                        128);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarSetTriggerOperation failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }

    // TODO: Select external trigger parameters as required

    retCode = AlazarSetExternalTrigger(boardHandle,
                                       DC_COUPLING,
                                       ETR_5V);

    // TODO: Set trigger delay as required.

    double triggerDelay_sec = 0;
    U32 triggerDelay_samples = (U32)(triggerDelay_sec * samplesPerSec + 0.5);
    retCode = AlazarSetTriggerDelay(boardHandle, triggerDelay_samples);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarSetTriggerDelay failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }

    // TODO: Set trigger timeout as required.

    // NOTE:
    //
    // The board will wait for a for this amount of time for a trigger event. If a trigger event
    // does not arrive, then the board will automatically trigger. Set the trigger timeout value to
    // 0 to force the board to wait forever for a trigger event.
    //
    // IMPORTANT:
    //
    // The trigger timeout value should be set to zero after appropriate trigger parameters have
    // been determined, otherwise the board may trigger if the timeout interval expires before a
    // hardware trigger event arrives.

    retCode = AlazarSetTriggerTimeOut(boardHandle, 0);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarSetTriggerTimeOut failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }

    // TODO: Configure AUX I/O connector as required

    retCode = AlazarConfigureAuxIO(boardHandle, AUX_OUT_TRIGGER, 0);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarConfigureAuxIO failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }

    return TRUE;

}

bool AlazarControlThread::AcquireData(HANDLE * boardHandleArray)
{
    // There are no pre-trigger samples in NPT mode
    //U32 preTriggerSamples = 0;
    preTriggerSamples = PRE_TRIGGER_SAMPLES; //JATS CHANGE: making these accessible from other functions

    // TODO: Select the number of post-trigger samples per record
    //postTriggerSamples = 256;
    postTriggerSamples = POST_TRIGGER_SAMPLES; //JATS CHANGE: making these accessible from other functions

    // TODO: Specify the number of records per DMA buffer
    //U32 recordsPerBuffer = 2000;
    recordsPerBuffer = RECORDS_PER_BUFFER; //JATS CHANGE: making these accessible from other functions

    // TODO: Specify the total number of buffers to capture
    buffersPerAcquisition = BUFFERS_PER_ACQUISITION;

    // TODO: Select which channels to capture (A, B, or both)
    U32 channelMask = CHANNEL_A | CHANNEL_B;

    HANDLE systemHandle = boardHandleArray[0];

    // Calculate the number of enabled channels from the channel mask
    int channelCount = 0;
    int channelsPerBoard = 2;
    for (int channel = 0; channel < channelsPerBoard; channel++)
    {
        U32 channelId = 1U << channel;
        if (channelMask & channelId)
            channelCount++;
    }

    // Get the sample size in bits, and the on-board memory size in samples per channel
    U8 bitsPerSample;
    U32 maxSamplesPerChannel;
    RETURN_CODE retCode = AlazarGetChannelInfo(systemHandle, &maxSamplesPerChannel, &bitsPerSample);
    if (retCode != ApiSuccess)
    {
        printf("Error: AlazarGetChannelInfo failed -- %s\n", AlazarErrorToText(retCode));
        return FALSE;
    }

    // Calculate the size of each DMA buffer in bytes
    float bytesPerSample = (float) ((bitsPerSample + 7) / 8);
    U32 samplesPerRecord = preTriggerSamples + postTriggerSamples;
    U32 bytesPerRecord = (U32)(bytesPerSample * samplesPerRecord +
                               0.5); // 0.5 compensates for double to integer conversion
    bytesPerBuffer = bytesPerRecord * recordsPerBuffer * channelCount;

    // JATS CHANGE: Add in a saveBuffer which is specified as the number of buffers per acquisition * samples per buffer

    saveBuffer = new U16[2*bytesPerBuffer/2*buffersPerAcquisition];
    waitTimeBuffer = new double[buffersPerAcquisition*2];

    // JATS CHANGE: Add in sendEmit flag to help GUI only receive emits when it is ready
    bool sendEmit = false;
    std::atomic<bool> sendEmitAtom = false;

    BOOL success = TRUE;
    // Allocate memory for DMA buffers
    U32 bufferIndex;
    U32 boardIndex;
    for (boardIndex = 0; boardIndex < 2 && success; boardIndex++) {
        for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT) && success; bufferIndex++)
        {
            // Allocate page aligned memory
            BufferArray[boardIndex][bufferIndex] =
                (U16 *)AlazarAllocBufferU16(boardHandleArray[boardIndex], bytesPerBuffer);
            if (BufferArray[boardIndex][bufferIndex] == NULL)
            {
                printf("Error: Alloc %u bytes failed\n", bytesPerBuffer);
                success = FALSE;
            }
        }
    }

    // Prepare each board for an AutoDMA acquisition
    for (boardIndex = 0; boardIndex < 2 && success; boardIndex++){
        // Configure the record size
        if (success)
        {
            retCode = AlazarSetRecordSize(boardHandleArray[boardIndex], preTriggerSamples, postTriggerSamples);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarSetRecordSize failed -- %s\n", AlazarErrorToText(retCode));
                success = FALSE;
            }
        }


        // JATS CHANGE: Checking NPT requirements
    //    U32 retValue;
    //    retCode = AlazarQueryCapability(boardHandle,CAP_MAX_NPT_PRETRIGGER_SAMPLES,0, &retValue);

        // Configure the board to make an NPT AutoDMA acquisition
        if (success)
        {
            //U32 recordsPerAcquisition = recordsPerBuffer * buffersPerAcquisition;
            U32 infiniteRecords = 0x7FFFFFFF; // Acquire until aborted or timeout. // JATS CHANGE: want infinite acquisition
            U32 admaFlags = ADMA_EXTERNAL_STARTCAPTURE | ADMA_NPT;
            retCode = AlazarBeforeAsyncRead(boardHandleArray[boardIndex], channelMask, -(long)preTriggerSamples,
                                            samplesPerRecord, recordsPerBuffer, infiniteRecords,
                                            admaFlags);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarBeforeAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
                success = FALSE;
            }
        }

        // Add the buffers to a list of buffers available to be filled by the board

        for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT) && success; bufferIndex++)
        {
            U16 *pBuffer = BufferArray[boardIndex][bufferIndex];
            retCode = AlazarPostAsyncBuffer(boardHandleArray[boardIndex], pBuffer, bytesPerBuffer);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarPostAsyncBuffer %u failed -- %s\n", bufferIndex,
                       AlazarErrorToText(retCode));
                success = FALSE;
            }
        }
    }

    // Arm the board system to wait for a trigger event to begin the acquisition
    if (success)
    {
        retCode = AlazarStartCapture(systemHandle);
        if (retCode != ApiSuccess)
        {
            printf("Error: AlazarStartCapture failed -- %s\n", AlazarErrorToText(retCode));
            success = FALSE;
        }
    }

    // Wait for each buffer to be filled, process the buffer, and re-post it to
    // the board.
    if (success)
    {
        //JATS CHANGE: printf("Capturing %d buffers ... press any key to abort\n", buffersPerAcquisition);

        U32 startTickCount = GetTickCount();
        U32 buffersCompleted = 0;
        INT64 bytesTransferred = 0;

        while (running)
        {
            // TODO: Set a buffer timeout that is longer than the time
            //       required to capture all the records in one buffer.
            U32 timeout_ms = 100000;

            // Wait for the buffer at the head of the list of available buffers
            // to be filled by the board.
            bufferIndex = buffersCompleted % BUFFER_COUNT;

            for (boardIndex = 0; (boardIndex < 2) && success; boardIndex++){

                U16 *pBuffer = BufferArray[boardIndex][bufferIndex];

                retCode = AlazarWaitAsyncBufferComplete(boardHandleArray[boardIndex], pBuffer, timeout_ms);

                if (retCode != ApiSuccess)
                {
                    qDebug() << "Error: AlazarWaitAsyncBufferComplete failed --" << AlazarErrorToText(retCode);
                    success = FALSE;
                }

                if (success && (!pauseSaveBuffer))
                {
                    U64 index = 0;
                    if (boardIndex == 0){
                        index = (numSaveBufferAtom%buffersPerAcquisition)*2*bytesPerBuffer/2;
                    }
                    else {
                        index = (numSaveBufferAtom%buffersPerAcquisition)*2*bytesPerBuffer/2+1*bytesPerBuffer/2;
                    }
                    memmove(&(saveBuffer[index]),pBuffer,bytesPerBuffer);

                    if (boardIndex == 1){
                        numSaveBufferAtom++;
                        sendEmitAtom = !flagAtom;
                        flagAtom = true;

                        if (sendEmitAtom){
                            //qDebug() << "Emit sent on buffer number" << numSaveBufferAtom;
                            emit dataReady(this);
                        }
                        else {
                            //qDebug() << "No emit on buffer #" << numSaveBufferAtom;
                        }
                    }

                    if (saveData)
                    {
                        if (currentSaveCount < totalSaveCount){
                            // Write record to file
                            if(boardIndex ==1){
                                currentSaveCount++;
                            }

                            size_t bytesWritten;
                            if (fwrite != NULL){
                                bytesWritten = fwrite(pBuffer, sizeof(BYTE), bytesPerBuffer, continuousSaveFile);
                            }
                            else {
                                qDebug() << "File handle is null on trying to write before currentSaveCount < totalSaveCount";
                                stopContinuousSave();
                            }

                            if (bytesWritten != bytesPerBuffer)
                            {
                                qDebug() << "Error: Write buffer" << numSaveBufferAtom << "failed --" << GetLastError();
                                stopContinuousSave();
                            }
                            qDebug() << "Buffer number " << currentSaveCount << "saved successfully";
                        }
                        else {
                            stopContinuousSave();
                        }
                    }
                }

                // Add the buffer to the end of the list of available buffers.
                if (success)
                {
                    retCode = AlazarPostAsyncBuffer(boardHandleArray[boardIndex], pBuffer, bytesPerBuffer);
                    if (retCode != ApiSuccess)
                    {
                        qDebug() << "Error: AlazarPostAsyncBuffer failed --" << AlazarErrorToText(retCode);
                        success = FALSE;
                    }
                }
            }
            // If the acquisition failed, exit the acquisition loop
            if (!success){
                qDebug() << "Acquisition has failed. Exiting acquisition loop...";
                break;
            }
            buffersCompleted++;
            bytesTransferred += bytesPerBuffer;

            // Display progress
            //qDebug() << buffersCompleted;

        }

        // Display results
        double transferTime_sec = (GetTickCount() - startTickCount) / 1000.;
        //printf("Capture completed in %.2lf sec\n", transferTime_sec);

        double buffersPerSec;
        double bytesPerSec;
        double recordsPerSec;
        U32 recordsTransferred = recordsPerBuffer * buffersCompleted;

        if (transferTime_sec > 0.)
        {
            buffersPerSec = buffersCompleted / transferTime_sec;
            bytesPerSec = bytesTransferred / transferTime_sec;
            recordsPerSec = recordsTransferred / transferTime_sec;
        }
        else
        {
            buffersPerSec = 0.;
            bytesPerSec = 0.;
            recordsPerSec = 0.;
        }

//        printf("Captured %u buffers (%.4g buffers per sec)\n", buffersCompleted, buffersPerSec);
//        printf("Captured %u records (%.4g records per sec)\n", recordsTransferred, recordsPerSec);
//        printf("Transferred %I64d bytes (%.4g bytes per sec)\n", bytesTransferred, bytesPerSec);

    }

    // Abort the acquisition
    for(boardIndex = 0; boardIndex < 2; boardIndex++){
        retCode = AlazarAbortAsyncRead(boardHandleArray[boardIndex]);
        if (retCode != ApiSuccess)
        {
    //        printf("Error: AlazarAbortAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
            //qDebug() << "Error: AlazarAbortAsyncRead failed -- " << AlazarErrorToText(retCode);
            success = FALSE;
        }
    }
    // Free all memory allocated
    for(boardIndex = 0; boardIndex < 2; boardIndex++){
        for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
        {
            if (BufferArray[bufferIndex] != NULL)
            {
                AlazarFreeBufferU16(boardHandleArray[boardIndex], BufferArray[boardIndex][bufferIndex]);
            }
        }
    }

    qDebug() << "DTh: Data acquisition run loop finished...";
    return success;
}

void AlazarControlThread::readLatestData(QVector< QVector<double> > *ch1,
                                         QVector< QVector<double> > *ch2,
                                         QVector< QVector<double> > *ch3,
                                         QVector< QVector<double> > *ch4)
{
    //U32 startTickCount = GetTickCount();
    U64 totalSamplesPerChannelPerRecord = preTriggerSamples + postTriggerSamples;
    U32 tempBuffersCompleted = 0;

    tempBuffersCompleted = numSaveBufferAtom;
    flagAtom = false;

    U16 * windowBuffer = saveBuffer+((tempBuffersCompleted-1)%buffersPerAcquisition)*2*bytesPerBuffer/2;
    U16 offset = 0x8000;

    // *(1/8192) respresents the scaling of this range from -32768 to 32768 to -1 to 1
    // *voltageRange represents the scaling of -1 to 1 to -Voltage max to +Voltage max
    double ch1_scale = 1.25*(1/32768.0);
    double ch2_scale = 1.25*(1/32768.0);
    double ch3_scale = 1.25*(1/32768.0);
    double ch4_scale = 1.25*(1/32768.0);

//    U64 index_offset;
//    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
//        // could put a dereferencing section here for the first loop for two dimensional vectors to save some compute time
//        index_offset = i*totalSamplesPerChannelPerRecord*4;
//        for (int j = 0; j < totalSamplesPerChannelPerRecord; j++){
//            (*ch1)[i][j] = ch1_scale*(windowBuffer[j*4+0+index_offset]-offset);
//            (*ch2)[i][j] = ch2_scale*(windowBuffer[j*4+1+index_offset]-offset);
//            (*ch3)[i][j] = ch3_scale*(windowBuffer[j*4+2+index_offset]-offset);
//            (*ch4)[i][j] = ch4_scale*(windowBuffer[j*4+3+index_offset]-offset);
//        }
//    }

    U64 index_offset;
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        index_offset = i*totalSamplesPerChannelPerRecord;
        for (int j = 0; j < totalSamplesPerChannelPerRecord; j++){
            (*ch1)[i][j] = ch1_scale*(windowBuffer[j+index_offset+RECORDS_PER_BUFFER*0*totalSamplesPerChannelPerRecord]-offset);
            (*ch2)[i][j] = ch2_scale*(windowBuffer[j+index_offset+RECORDS_PER_BUFFER*1*totalSamplesPerChannelPerRecord]-offset);
            (*ch3)[i][j] = ch3_scale*(windowBuffer[j+index_offset+RECORDS_PER_BUFFER*2*totalSamplesPerChannelPerRecord]-offset);
            (*ch4)[i][j] = ch4_scale*(windowBuffer[j+index_offset+RECORDS_PER_BUFFER*3*totalSamplesPerChannelPerRecord]-offset);
        }
    }
    //double transferTime_sec = (GetTickCount() - startTickCount) / 1000.;
    //qDebug() << transferTime_sec;
}

// stopRunning function should set the running boolean to false to allow for a clean exit from the the alazartech control thread
void AlazarControlThread::stopRunning()
{
    running = false;
}

// InputRangeIdToVolts function taken from AlazarTech codebase to translate an input range id to a voltage double
double AlazarControlThread::InputRangeIdToVolts(U32 inputRangeId)
{
    double inputRange_volts;

    switch (inputRangeId)
    {
    case INPUT_RANGE_PM_20_MV:
        inputRange_volts = 20.e-3;
        break;
    case INPUT_RANGE_PM_40_MV:
        inputRange_volts = 40.e-3;
        break;
    case INPUT_RANGE_PM_50_MV:
        inputRange_volts = 50.e-3;
        break;
    case INPUT_RANGE_PM_80_MV:
        inputRange_volts = 80.e-3;
        break;
    case INPUT_RANGE_PM_100_MV:
        inputRange_volts = 100.e-3;
        break;
    case INPUT_RANGE_PM_200_MV:
        inputRange_volts = 200.e-3;
        break;
    case INPUT_RANGE_PM_400_MV:
        inputRange_volts = 400.e-3;
        break;
    case INPUT_RANGE_PM_500_MV:
        inputRange_volts = 500.e-3;
        break;
    case INPUT_RANGE_PM_800_MV:
        inputRange_volts = 800.e-3;
        break;
    case INPUT_RANGE_PM_1_V:
        inputRange_volts = 1.;
        break;
    case INPUT_RANGE_PM_2_V:
        inputRange_volts = 2.;
        break;
    case INPUT_RANGE_PM_4_V:
        inputRange_volts = 4.;
        break;
    case INPUT_RANGE_PM_5_V:
        inputRange_volts = 5.;
        break;
    case INPUT_RANGE_PM_8_V:
        inputRange_volts = 8.;
        break;
    case INPUT_RANGE_PM_10_V:
        inputRange_volts = 10.;
        break;
    case INPUT_RANGE_PM_20_V:
        inputRange_volts = 20.;
        break;
    case INPUT_RANGE_PM_40_V:
        inputRange_volts = 40.;
        break;
    case INPUT_RANGE_PM_16_V:
        inputRange_volts = 16.;
        break;
    case INPUT_RANGE_HIFI:
        inputRange_volts = 0.525;
        break;
    default:
        printf("Error: Invalid input range %u\n", inputRangeId);
        inputRange_volts = -1.;
        break;
    }

    return inputRange_volts;
}

// saveDataBuffer function should pause writing to the saveBuffer, open a file for writing,
// write the current buffer to a file, close file, restart writing to the saveBuffer, and return 0
int AlazarControlThread::saveDataBuffer()
{
    // Pause system saveBuffer write
    pauseSaveBuffer = true;
    qDebug() << "Starting file save...";
    FILE *fpData = NULL;
    size_t bytesWritten;

    //find date and time and concatenate
    time_t t = time(NULL);
    struct tm buff;
    localtime_s(&buff,&t);
    char dateTime[100];
    std::strftime(dateTime,sizeof(dateTime),"%Y-%m-%d-%H-%M-%S",&buff);
    std::string fileName = SAVE_PATH;
    fileName = fileName + dateTime + "-data.bin";

    // open file
    fpData = fopen(fileName.c_str(), "wb");
    if (fpData == NULL)
    {
        qDebug() << "Error: Unable to create data file --" << GetLastError();
        return 1;
    }
    qDebug() << "Successfully created data file";

//    U32 tempBuffersCompleted = 0;
//    {
//        QMutexLocker locker(&mutex);
//        tempBuffersCompleted = numberOfBuffersAddedToSaveBuffer;
//    }

    U32 tempBuffersCompleted = numSaveBufferAtom;
    if (tempBuffersCompleted < BUFFERS_PER_ACQUISITION){
        bytesWritten= fwrite(saveBuffer, sizeof(BYTE), tempBuffersCompleted*2*bytesPerBuffer, fpData);

        // check if errored
        if (bytesWritten != tempBuffersCompleted*2*bytesPerBuffer){
            qDebug() << "Error: Write buffer failed --";
            qDebug() << "Bytes written: " << bytesWritten << "does not equal" << tempBuffersCompleted*2*bytesPerBuffer;
            if (fpData != NULL)
                fclose(fpData);
            return 1;
        }
    }
    else{
        // we have an array with [[5] [2] [3] [4]]
        // buffersCompleted is 5, so 5%4 is 1
        // so first fwrite from buffersCompleted to end of file
        bytesWritten = fwrite(&saveBuffer[(tempBuffersCompleted%BUFFERS_PER_ACQUISITION)*(2*bytesPerBuffer/2)],
                              sizeof(BYTE),
                              (BUFFERS_PER_ACQUISITION-(tempBuffersCompleted%BUFFERS_PER_ACQUISITION))*2*bytesPerBuffer,
                              fpData);
        if ((tempBuffersCompleted%BUFFERS_PER_ACQUISITION) != 0){
            // if the buffer does not fit perfectly,
            // then fwrite from beginning of file to buffersCompleted
            bytesWritten += fwrite(saveBuffer,
                                   sizeof(BYTE),
                                   ((tempBuffersCompleted%BUFFERS_PER_ACQUISITION))*2*bytesPerBuffer,
                                   fpData);
        }

        // check if errored
        if (bytesWritten != BUFFERS_PER_ACQUISITION*2*bytesPerBuffer){
            qDebug() << "Error: Write buffer failed";
            qDebug() << "Bytes written: " << bytesWritten << "does not equal" << BUFFERS_PER_ACQUISITION*2*bytesPerBuffer;
            if (fpData != NULL)
                fclose(fpData);
            return 1;
        }
    }

    // Close file
    if (fpData != NULL)
        fclose(fpData);

    /* COMMENT IN FOR TIME STAMPS
    // Write the time stamp buffer
    fpData = fopen("timeStamp.bin", "wb");
    bytesWritten = 0;

    if (tempBuffersCompleted < BUFFERS_PER_ACQUISITION){
        bytesWritten = fwrite(waitTimeBuffer,sizeof(BYTE),tempBuffersCompleted*8*2,fpData);
        if (bytesWritten != tempBuffersCompleted*8*2){
            qDebug() << "Error: Write buffer failed --";
            if (fpData != NULL)
                fclose(fpData);
            return 1;
        }
    }
    else{
        bytesWritten = fwrite(&waitTimeBuffer[(tempBuffersCompleted%BUFFERS_PER_ACQUISITION)*2],
                                sizeof(BYTE),
                                (BUFFERS_PER_ACQUISITION-(tempBuffersCompleted%BUFFERS_PER_ACQUISITION))*8*2,
                                fpData);
        if ((tempBuffersCompleted%BUFFERS_PER_ACQUISITION) != 0){
            // if the buffer does not fit perfectly,
            // then fwrite from beginning of file to buffersCompleted
            bytesWritten += fwrite(waitTimeBuffer,
                                   sizeof(BYTE),
                                   ((tempBuffersCompleted%BUFFERS_PER_ACQUISITION))*8*2,
                                   fpData);
        }

        if (bytesWritten != BUFFERS_PER_ACQUISITION*8*2){
            qDebug() << "Error: Write buffer failed";
            qDebug() << "Bytes written: " << bytesWritten << "does not equal" << BUFFERS_PER_ACQUISITION*bytesPerBuffer;
            if (fpData != NULL)
                fclose(fpData);
            return 1;
        }
    }

    // Close file
    if (fpData != NULL)
        fclose(fpData);
    */
    // Unpause system
    pauseSaveBuffer = false;
    qDebug() << "File save finished successfully";
    return 0;
}

// startContinuousSave function should create a file handle, open it for writing, set the saveData flag to true
// should double check that file is opened and if not call stopContinuousSave()
// CHANGES: this should change to having an input passed which in some way defines the number of buffes to acquire
void AlazarControlThread::startContinuousSave(int numBuffer)
{
    // initialize counters for saving
    totalSaveCount = numBuffer;
    currentSaveCount = 0;
    continuousSaveFile = NULL;

    // generate file name based on date and time
    time_t t = time(NULL);
    struct tm buff;
    localtime_s(&buff,&t);
    char dateTime[100];
    std::strftime(dateTime,sizeof(dateTime),"%Y-%m-%d-%H-%M-%S",&buff);
    std::string fileName = SAVE_PATH;
    fileName = fileName + dateTime + "-data.bin";

    // open file
    continuousSaveFile = fopen(fileName.c_str(), "wb");
    if (continuousSaveFile == NULL)
    {
        qDebug() << "Error: Unable to create data file --" << GetLastError();
        stopContinuousSave();
        return;
    }

    // enable writing to the file
    saveData = true;
    qDebug() << saveData;
    return;
}

// stopContinouSave function should close the file handle, call signal to inform GUI file save finished
// -- not clear on how to report to the front end whether the save was successful rn happening through console outputs
void AlazarControlThread::stopContinuousSave()
{
    // check if the file was opened successfully and if so close it
    saveData = false;
    if (continuousSaveFile != NULL)
        fclose(continuousSaveFile);
    emit continuousSaveComplete();
    return;
}


//#ifndef WIN32
//inline U32 GetTickCount(void)
//{
//    struct timeval tv;
//    if (gettimeofday(&tv, NULL) != 0)
//        return 0;
//    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
//}

//inline void Sleep(U32 dwTime_ms)
//{
//    usleep(dwTime_ms * 1000);
//}

//inline int _kbhit (void)
//{
//  struct timeval tv;
//  fd_set rdfs;

//  tv.tv_sec = 0;
//  tv.tv_usec = 0;

//  FD_ZERO(&rdfs);
//  FD_SET (STDIN_FILENO, &rdfs);

//  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
//  return FD_ISSET(STDIN_FILENO, &rdfs);
//}

//inline int GetLastError()
//{
//    return errno;
//}
//#endif
