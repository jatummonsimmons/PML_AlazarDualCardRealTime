#ifndef ACQUISITIONCONFIG_H
#define ACQUISITIONCONFIG_H

// Acquisition parameters
#define PRE_TRIGGER_SAMPLES (128)
//#define POST_TRIGGER_SAMPLES (512)
#define POST_TRIGGER_SAMPLES (896)
#define RECORDS_PER_BUFFER (1000)
#define BUFFERS_PER_ACQUISITION (1000)

// Processing parameters
#define NUM_AVERAGE_SIGNALS (20)
#define NUM_PIXELS (200)
#define MIRROR_VOLTAGE_RANGE_PM_V (0.2) // FOR PLOTTING PURPOSES ONLY

// Save parameters
#define SAVE_PATH ("C:\\Users\\illumiSonics\\Documents\\JATS_TempDataRecord\\")

#endif // ACQUISITIONCONFIG_H
