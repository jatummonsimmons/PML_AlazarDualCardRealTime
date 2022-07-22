#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    ch1(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch2(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch3(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch4(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    x(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    sig_m1(RECORDS_PER_BUFFER),
    sig_m2(RECORDS_PER_BUFFER),
    sig_sc(RECORDS_PER_BUFFER),
    sig_gsc(RECORDS_PER_BUFFER),
    sig_pa(RECORDS_PER_BUFFER),
    y(RECORDS_PER_BUFFER),
    first_time(true),
    background_collected(false),
    background_subtract(false),
    startup(true)
{
    for (int i = 0; i < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES;i++){
        x[i] = i;
    }
    for (int i =0;i<RECORDS_PER_BUFFER;i++){
        y[i] = i;
    }
    ui->setupUi(this);

    // Setting up image parameters
    colorMap_sc = new QCPColorMap(ui->customPlot_img_sc->xAxis, ui->customPlot_img_sc->yAxis);
    colorMap_pa = new QCPColorMap(ui->customPlot_img_pa->xAxis, ui->customPlot_img_pa->yAxis);
    colorMap_gsc = new QCPColorMap(ui->customPlot_img_gsc->xAxis, ui->customPlot_img_gsc->yAxis);
    colorMap_sc_gsc = new QCPColorMap(ui->customPlot_img_sc_gsc->xAxis, ui->customPlot_img_sc_gsc->yAxis);

    // Setting img data arrays to zero
    memset(img_count,0,sizeof(img_count));
    memset(img_sc,0,sizeof(img_sc));
    memset(img_gsc,0,sizeof(img_gsc));
    memset(img_pa,0,sizeof(img_pa));

    memset(imgAcc_sc,0,sizeof(img_sc));
    memset(imgAcc_gsc,0,sizeof(img_gsc));
    memset(imgAcc_pa,0,sizeof(img_pa));

    memset(img_sc_background,0,sizeof(img_sc));
    memset(img_gsc_background,0,sizeof(img_gsc));

    // Setting up image plots
    setupScImgPlot(ui->customPlot_img_sc);
    setupPaImgPlot(ui->customPlot_img_pa);
    setupGScImgPlot(ui->customPlot_img_gsc);
    setupScGScImgPlot(ui->customPlot_img_sc_gsc);

/* Commented out to simplify UI
    // Setting up time domain plots
    setupTimeDomainPlot(ui->customPlot_1);
    setupTimeDomainPlot(ui->customPlot_2);
    setupTimeDomainPlot(ui->customPlot_3);
    setupTimeDomainPlot(ui->customPlot_4);
*/

    // Setting up average time domain plot
    setupAvgSigPlot(ui->customPlot_avgSig);

/* Commented out to simplify UI
    // Setting up signal plots
    setupSigPlot(ui->customPlot_sig_m1);
    setupSigPlot(ui->customPlot_sig_m2);
    setupSigPlot(ui->customPlot_sig_sc);
    setupSigPlot(ui->customPlot_sig_pa);
*/

    // setting up signal vs. mirror plot
    setupSigVsMirrorPlot(ui->customPlot_sc_m1);
    setupSigVsMirrorPlot(ui->customPlot_pa_m1);
    setupSigVsMirrorPlot(ui->customPlot_gsc_m1);

    // setting up sc vs gsc vs mirror plot
    setupScVsGscVsMirrorPlot(ui->customPlot_sc_gsc);

    //Initialization connections between control thread and processing thread
    connect(&dataThread,&AlazarControlThread::dataReady,
            &processingThread, &dataProcessingThread::updateTimeDomains);

    // Initialize connections between processing thread and front end thread
    connect(&processingThread,&dataProcessingThread::avgSig_ready,
            this,&MainWindow::updateAvgSig);

    connect(&processingThread,&dataProcessingThread::sig_ready,
            this,&MainWindow::updateSig);

    // Initialize connections between front end thread and dataThread
    connect(&dataThread,&AlazarControlThread::continuousSaveComplete,
            this,&MainWindow::continuousSaveComplete);

    // Initialize lineEdit validator to ensure sensible entries
    ui->lineEdit->setValidator( new QIntValidator(0, 10000, this) );
    ui->lineEdit->setText("500");

    // Start the threads moving
    processingThread.start(QThread::NormalPriority);
    dataThread.start(QThread::TimeCriticalPriority);
}

MainWindow::~MainWindow()
{
    qDebug() << "GUI: MainWindow destructor called...";
    processingThread.exit();
    dataThread.exit();

    delete [] colorMap_sc;
    delete [] colorMap_gsc;
    delete [] colorMap_pa;
    delete [] colorMap_sc_gsc;

    delete ui;
    qDebug() << "GUI: MainWindow destructor finished...";
}

//void MainWindow::updateTimeDomain()
//{
//    processingThread.read_rawSig(&ch1,&ch2,&ch3,&ch4);
//    ui->customPlot_1->graph(0)->setData(x,ch1);
//    ui->customPlot_2->graph(0)->setData(x,ch2);
//    ui->customPlot_3->graph(0)->setData(x,ch3);
//    ui->customPlot_4->graph(0)->setData(x,ch4);
//    ui->customPlot_1->replot();
//    ui->customPlot_2->replot();
//    ui->customPlot_3->replot();
//    ui->customPlot_4->replot();
//}

void MainWindow::updateAvgSig()
{
    processingThread.read_avgSig(&avgSig);
    for(int i = 0; i < NUM_AVERAGE_SIGNALS;i++){
        ui->customPlot_avgSig->graph(i)->setData(x,avgSig[i]);
    }
    ui->customPlot_avgSig->replot();
}

void MainWindow::updateSig()
{
    processingThread.read_sig(&sig_pa,&sig_sc,&sig_gsc,&sig_m1,&sig_m2);

    // Update extracted signal vs. buffer
//    ui->customPlot_sig_pa->graph(0)->setData(y,sig_pa);
//    ui->customPlot_sig_sc->graph(0)->setData(y,sig_sc);
//    ui->customPlot_sig_m1->graph(0)->setData(y,sig_m1);
//    ui->customPlot_sig_m2->graph(0)->setData(y,sig_m2);
//    ui->customPlot_sig_pa->replot();
//    ui->customPlot_sig_sc->replot();
//    ui->customPlot_sig_m1->replot();
//    ui->customPlot_sig_m2->replot();

    // Update all images
    int temp_x_index;
    int temp_y_index;    

    for (int i = 0; i < sig_m1.length(); i++)
    {
        // Convert from floating point representing position to index in image
        colorMap_sc->data()->coordToCell(sig_m1[i], sig_m2[i], &temp_x_index, &temp_y_index);

        // Error handling in both x and y to confirm that the index is within frame
        if (temp_x_index < 0){
            temp_x_index = 0;
        }
        if (temp_x_index >=NUM_PIXELS){
            temp_x_index = NUM_PIXELS-1;
        }
        if (temp_y_index < 0){
            temp_y_index = 0;
        }
        if (temp_y_index >=NUM_PIXELS){
            temp_y_index = NUM_PIXELS - 1;
        }

        // Update accumulator arrays
        img_count[temp_x_index][temp_y_index] +=1;
        imgAcc_sc[temp_x_index][temp_y_index] += sig_sc[i];
        imgAcc_gsc[temp_x_index][temp_y_index] += sig_gsc[i];
        imgAcc_pa[temp_x_index][temp_y_index] += sig_pa[i];

        // Update image arrays
        img_sc[temp_x_index][temp_y_index] = imgAcc_sc[temp_x_index][temp_y_index] / img_count[temp_x_index][temp_y_index];
        img_gsc[temp_x_index][temp_y_index] = imgAcc_gsc[temp_x_index][temp_y_index] / img_count[temp_x_index][temp_y_index];
        img_pa[temp_x_index][temp_y_index] = imgAcc_pa[temp_x_index][temp_y_index] / img_count[temp_x_index][temp_y_index];

    }

    // Logic to keep track of and clear lines of the image as the system scans
    double sc_max, sc_min;
    double gsc_max, gsc_min;

    if (startup){
        last_y_index = temp_y_index;
        startup = false;
    }

    if (abs(temp_y_index - last_y_index) > 1 ){
        int erase_line_1;
        int erase_line_2;

        int offset = 2;
        if (((NUM_PIXELS - 1) - temp_y_index) < offset + 1){ // if I am within offset + 1 of the bottom of the image, always delete the lines around
            erase_line_1 = (NUM_PIXELS - 1) - offset;
            erase_line_2 = (NUM_PIXELS - 1) - offset - 1;
        }
        else if (temp_y_index < offset + 1){ // if I am within offset + 1 of the top of the image, always delete the lines around
            erase_line_1 = offset;
            erase_line_2 = offset + 1;
        }
        else if (temp_y_index - last_y_index < 0){ // if my last y index is bigger than my current y index, ie. we are heading up the image, delete two rows in front of me, ie. up higher on the image
            erase_line_1 = temp_y_index - offset;
            erase_line_2 = temp_y_index - offset - 1;
        }
        else if (temp_y_index - last_y_index > 0){ // if my last y index is smaller than my current y index, ie. we are heading down the image, delete two rows in front of me, ie. lower down on the image
            erase_line_1 = temp_y_index + offset;
            erase_line_2 = temp_y_index + offset + 1;
        }

        // run through the selected row and reset values to zero
        for (int i = 0; i < NUM_PIXELS; i++){
            colorMap_sc->data()->setCell(i,erase_line_1,0);
            colorMap_pa->data()->setCell(i,erase_line_1,0);
            colorMap_gsc->data()->setCell(i,erase_line_1,0);

            img_count[i][erase_line_1] = 0;
            img_sc[i][erase_line_1] = 0;
            img_pa[i][erase_line_1] = 0;
            img_gsc[i][erase_line_1] = 0;
            imgAcc_sc[i][erase_line_1] = 0;
            imgAcc_pa[i][erase_line_1] = 0;
            imgAcc_gsc[i][erase_line_1] = 0;
        }

        for (int i = 0; i < NUM_PIXELS; i++){
            colorMap_sc->data()->setCell(i,erase_line_2,0);
            colorMap_pa->data()->setCell(i,erase_line_2,0);
            colorMap_gsc->data()->setCell(i,erase_line_2,0);

            img_count[i][erase_line_2] = 0;
            img_sc[i][erase_line_2] = 0;
            img_pa[i][erase_line_2] = 0;
            img_gsc[i][erase_line_2] = 0;
            imgAcc_sc[i][erase_line_2] = 0;
            imgAcc_pa[i][erase_line_2] = 0;
            imgAcc_gsc[i][erase_line_2] = 0;
        }

        // find max / min of NIR scattering image
        bool first = true;
        if (!background_subtract){
            for(int i = 0; i < NUM_PIXELS; i++){
                for(int j = 0; j < NUM_PIXELS; j++){
                    if (img_sc[i][j] != 0){
                        if (first){
                            sc_min = img_sc[i][j];
                            sc_max = img_sc[i][j];
                            first = false;
                        }
                        else {
                            if (img_sc[i][j] < sc_min){
                                sc_min = img_sc[i][j];
                            }
                            if (img_sc[i][j] > sc_max){
                                sc_max = img_sc[i][j];
                            }
                        }
                    }
                }
            }
        }
        else {
            for(int i = 0; i < NUM_PIXELS; i++){
                for(int j = 0; j < NUM_PIXELS; j++){
                    if ((img_gsc[i][j] != 0) && (img_gsc_background[i][j] != 0)){
                        if (first){
                            sc_min = (img_sc[i][j]-img_sc_background[i][j]);
                            sc_max = (img_sc[i][j]-img_sc_background[i][j]);
                            first = false;
                        }
                        else {
                            if ((img_sc[i][j]-img_sc_background[i][j]) < sc_min){
                                sc_min = (img_sc[i][j]-img_sc_background[i][j]);
                            }
                            if ((img_sc[i][j]-img_sc_background[i][j]) > sc_max){
                                sc_max = (img_sc[i][j]-img_sc_background[i][j]);
                            }
                        }
                    }
                }
            }
        }

        // find max / min of excitation scattering image
        first = true;
        if (!background_subtract){
            for(int i = 0; i < NUM_PIXELS; i++){
                for(int j = 0; j < NUM_PIXELS; j++){
                    if (img_gsc[i][j] != 0){
                        if (first){
                            gsc_min = img_gsc[i][j];
                            gsc_max = img_gsc[i][j];
                            first = false;
                        }
                        else {
                            if (img_gsc[i][j] < gsc_min){
                                gsc_min = img_gsc[i][j];
                            }
                            if (img_gsc[i][j] > gsc_max){
                                gsc_max = img_gsc[i][j];
                            }
                        }
                    }
                }
            }
        }
        else {
            for(int i = 0; i < NUM_PIXELS; i++){
                for(int j = 0; j < NUM_PIXELS; j++){
                    if ((img_gsc[i][j] != 0) && (img_gsc_background[i][j] != 0)){
                        if (first){
                            gsc_min = (img_gsc[i][j]-img_gsc_background[i][j]);
                            gsc_max = (img_gsc[i][j]-img_gsc_background[i][j]);
                            first = false;
                        }
                        else {
                            if ((img_gsc[i][j]-img_gsc_background[i][j]) < gsc_min){
                                gsc_min = (img_gsc[i][j]-img_gsc_background[i][j]);
                            }
                            if ((img_gsc[i][j]-img_gsc_background[i][j]) > gsc_max){
                                gsc_max = (img_gsc[i][j]-img_gsc_background[i][j]);
                            }
                        }
                    }
                }
            }
        }

        // find max / min of PARS image
        double pa_max, pa_min;
        first = true;
        for(int i = 0; i < NUM_PIXELS; i++){
            for(int j = 0; j < NUM_PIXELS; j++){
                if (img_pa[i][j] != 0){
                    if (first){
                        pa_min = img_pa[i][j];
                        pa_max = img_pa[i][j];
                        first = false;
                    }
                    else {
                        if (img_pa[i][j] < pa_min){
                            pa_min = img_pa[i][j];
                        }
                        if (img_pa[i][j] > pa_max){
                            pa_max = img_pa[i][j];
                        }
                    }
                }
            }
        }

        // Update the images
        if (!background_subtract){
            for (int i = 0; i < NUM_PIXELS ; i++){
                for (int j = 0; j < NUM_PIXELS; j++){
                    colorMap_sc->data()->setCell(i,j,(img_sc[i][j]-sc_min)/(sc_max-sc_min));
                    colorMap_gsc->data()->setCell(i,j,(img_gsc[i][j]-gsc_min)/(gsc_max-gsc_min));
                    colorMap_pa->data()->setCell(i,j,img_pa[i][j]);
                    colorMap_sc_gsc->data()->setCell(i,j,colorMap_sc->data()->cell(i,j) - colorMap_gsc->data()->cell(i,j));
                }
            }
        }
        else{
            for (int i = 0; i < NUM_PIXELS ; i++){
                for (int j = 0; j < NUM_PIXELS; j++){
                    colorMap_sc->data()->setCell(i,j,((img_sc[i][j]-img_sc_background[i][j])-sc_min)/(sc_max-sc_min));
                    colorMap_gsc->data()->setCell(i,j,((img_gsc[i][j]- img_gsc_background[i][j])-gsc_min)/(gsc_max-gsc_min));
                    colorMap_pa->data()->setCell(i,j,img_pa[i][j]);
                    colorMap_sc_gsc->data()->setCell(i,j,colorMap_sc->data()->cell(i,j) - colorMap_gsc->data()->cell(i,j));
                }
            }
        }

        // Rescale image plots
       //colorMap_sc->setDataRange(QCPRange(sc_min,sc_max));
       //colorMap_gsc->setDataRange(QCPRange(gsc_min,gsc_max));

        double scale = 0.3;
        if (-pa_min > pa_max){
            colorMap_pa->setDataRange(QCPRange(scale*pa_min,-scale*pa_min));
        }
        else{
            colorMap_pa->setDataRange(QCPRange(-scale*pa_max,scale*pa_max));
        }

        ui->customPlot_img_sc->replot();
        ui->customPlot_img_gsc->replot();
        ui->customPlot_img_pa->replot();
        ui->customPlot_img_sc_gsc->replot();

        last_y_index = temp_y_index;

        if (!background_subtract){
            ui->customPlot_sc_m1->yAxis->setRange(sc_min,sc_max);
            ui->customPlot_gsc_m1->yAxis->setRange(gsc_min,gsc_max);
        }
        ui->customPlot_pa_m1->yAxis->setRange(pa_min,pa_max);

    }

    // Update extracted signal vs. fast axis
    ui->customPlot_sc_m1->graph(0)->setData(sig_m1,sig_sc);
    ui->customPlot_gsc_m1->graph(0)->setData(sig_m1,sig_gsc);
    ui->customPlot_pa_m1->graph(0)->setData(sig_m1,sig_pa);
    ui->customPlot_sc_m1->replot();
    ui->customPlot_pa_m1->replot();
    ui->customPlot_gsc_m1->replot();

    // Update Sc vs GSc

    bool first = true;
    double sc_buff_max, sc_buff_min;
    // Find max and min of both channels
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        if(first){
            sc_buff_max = sig_sc[i];
            sc_buff_min = sig_sc[i];
            first = false;
        }

        else{
            if(sig_sc[i] < sc_buff_min){
                sc_buff_min = sig_sc[i];
            }
            if(sig_sc[i] > sc_buff_max){
                sc_buff_max = sig_sc[i];
            }
        }
    }

    first = true;
    double gsc_buff_max, gsc_buff_min;
    // Find max and min of both channels
    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        if(first){
            gsc_buff_max = sig_gsc[i];
            gsc_buff_min = sig_gsc[i];
            first = false;
        }

        else{
            if(sig_gsc[i] < gsc_buff_min){
                gsc_buff_min = sig_gsc[i];
            }
            if(sig_gsc[i] > gsc_buff_max){
                gsc_buff_max = sig_gsc[i];
            }
        }
    }

    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        sig_sc[i] = (sig_sc[i]-sc_buff_min)/(sc_buff_max-sc_buff_min);
        sig_gsc[i] = (sig_gsc[i]-gsc_buff_min)/(gsc_buff_max-gsc_buff_min);
    }

    if(background_subtract){
        ui->customPlot_sc_m1->yAxis->setRange(sc_buff_min,sc_buff_max);
        ui->customPlot_gsc_m1->yAxis->setRange(gsc_buff_min,gsc_buff_max);
    }


    ui->customPlot_sc_gsc->graph(0)->setData(sig_m1,sig_sc);
    ui->customPlot_sc_gsc->graph(1)->setData(sig_m1,sig_gsc);
    ui->customPlot_sc_gsc->replot();

}

void MainWindow::setupTimeDomainPlot(QCustomPlot *customPlot)
{
    // Initialize all necessary customPlot parameters
    customPlot->addGraph();

    // give the axes some labels:
    customPlot->xAxis->setLabel("Time (Samples)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());

}

void MainWindow::setupScVsGscVsMirrorPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Position (V)");
    customPlot->yAxis->setLabel("Amplitude (0-1)");

    customPlot->xAxis->setRange(-MIRROR_VOLTAGE_RANGE_PM_V,MIRROR_VOLTAGE_RANGE_PM_V);
    customPlot->yAxis->setRange(0,1);

    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(170,1,20));
    customPlot->graph(1)->setPen(pen);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupAvgSigPlot(QCustomPlot *customPlot)
{
    for(int i = 0; i < NUM_AVERAGE_SIGNALS; i++){
        customPlot->addGraph();
    }
    customPlot->xAxis->setLabel("Time (Samples)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupSigPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Time (Records)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, RECORDS_PER_BUFFER);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupSigVsMirrorPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Position (V)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(-MIRROR_VOLTAGE_RANGE_PM_V,MIRROR_VOLTAGE_RANGE_PM_V);
    customPlot->yAxis->setRange(0,0.5);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupScImgPlot(QCustomPlot *customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);

    // set up the QCPColorMap:
    int nx = NUM_PIXELS;
    int ny = NUM_PIXELS;
    colorMap_sc->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap_sc->data()->setRange(QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V), QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_sc->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Scattering Amplitude (Voltage)");

    // set the color gradient of the color map to one of the presets:
    colorMap_sc->setGradient(QCPColorGradient::gpGrayscale);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    colorMap_sc->setDataRange(QCPRange(0,1));
    customPlot->rescaleAxes();

}

void MainWindow::setupPaImgPlot(QCustomPlot *customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);

    // set up the QCPColorMap:
    int nx = NUM_PIXELS;
    int ny = NUM_PIXELS;
    colorMap_pa->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap_pa->data()->setRange(QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V), QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_pa->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("PARS Amplitude (Voltage)");

    // set the color gradient of the color map to one of the presets:
    colorMap_pa->setGradient(QCPColorGradient::gpPolar);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    colorMap_pa->setDataRange(QCPRange(-0.10,0.10));
    customPlot->rescaleAxes();

}

void MainWindow::setupGScImgPlot(QCustomPlot *customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);

    // set up the QCPColorMap:
    int nx = NUM_PIXELS;
    int ny = NUM_PIXELS;
    colorMap_gsc->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap_gsc->data()->setRange(QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V), QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_gsc->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Green Scattering Amplitude (Voltage)");

    // set the color gradient of the color map to one of the presets:
    colorMap_gsc->setGradient(QCPColorGradient::gpGrayscale);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    colorMap_gsc->setDataRange(QCPRange(0,1));
    customPlot->rescaleAxes();
    customPlot->replot();

}

void MainWindow::setupScGScImgPlot(QCustomPlot *customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);

    // set up the QCPColorMap:
    int nx = NUM_PIXELS;
    int ny = NUM_PIXELS;
    colorMap_sc_gsc->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap_sc_gsc->data()->setRange(QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V), QCPRange(-MIRROR_VOLTAGE_RANGE_PM_V, MIRROR_VOLTAGE_RANGE_PM_V)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_sc_gsc->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Green Scattering Amplitude (Voltage)");

    // set the color gradient of the color map to one of the presets:
    colorMap_sc_gsc->setGradient(QCPColorGradient::gpPolar);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    colorMap_sc_gsc->setDataRange(QCPRange(-1,1));
    customPlot->rescaleAxes();
    customPlot->replot();

}

void MainWindow::continuousSaveComplete()
{
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    qDebug() << "Continuous save completed (GUI)";
}

void MainWindow::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    int success = dataThread.saveDataBuffer();

    ui->pushButton->setEnabled(true);
    if (success == 1){
        qDebug() << "(GUI) Save failed.";
    }
    else{
        qDebug() << "Save successful (GUI)";
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    QString numberOfBuffers = ui->lineEdit->text();
    int numBuffer = numberOfBuffers.toInt();

    if (numBuffer < 10){
        numBuffer = 10;
        ui->lineEdit->setText("10");
        qDebug() << "Minimum number of buffers for saving is 10.";
    }
    if (numBuffer > 10000){
        numBuffer = 10000;
        ui->lineEdit->setText("10000");
        qDebug() << "Maximum number of buffers for saving is 10000.";
    }

    qDebug() << numBuffer;
    dataThread.startContinuousSave(numBuffer);
}

void MainWindow::on_pushButton_3_clicked()
{
    //Kill thread and restart it
    qDebug() << "Stopping dataThread.";
    dataThread.stopRunning();
    forever{
        qDebug() << "Checking if finished.";

        if (dataThread.isFinished()){
            qDebug() << "Is finished!";
            break;
        }
        Sleep(10);
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    // Check line edit value
    QString rangeOfMirrors = ui->lineEdit_2->text();
    double mirrorRange = rangeOfMirrors.toDouble();

    // Update all image plots with this value
    colorMap_pa->data()->setRange(QCPRange(-mirrorRange, mirrorRange), QCPRange(-mirrorRange, mirrorRange)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions
    colorMap_sc->data()->setRange(QCPRange(-mirrorRange, mirrorRange), QCPRange(-mirrorRange, mirrorRange)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions
    colorMap_gsc->data()->setRange(QCPRange(-mirrorRange, mirrorRange), QCPRange(-mirrorRange, mirrorRange)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions
    colorMap_sc_gsc->data()->setRange(QCPRange(-mirrorRange, mirrorRange), QCPRange(-mirrorRange, mirrorRange)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    ui->customPlot_img_sc->rescaleAxes();
    ui->customPlot_img_pa->rescaleAxes();
    ui->customPlot_img_gsc->rescaleAxes();
    ui->customPlot_img_sc_gsc->rescaleAxes();

    // Update all mirror vs. plot with this value
    ui->customPlot_sc_m1->xAxis->setRange(-mirrorRange,mirrorRange);
    ui->customPlot_pa_m1->xAxis->setRange(-mirrorRange,mirrorRange);
    ui->customPlot_gsc_m1->xAxis->setRange(-mirrorRange,mirrorRange);
    ui->customPlot_sc_gsc->xAxis->setRange(-mirrorRange,mirrorRange);
}

// TO DO: Add monitoring for main window moves https://stackoverflow.com/questions/12953983/detect-end-of-movement-of-qmainwindow-qdialog-qt-4-8

void MainWindow::on_pushButton_5_clicked()
{
    for(int i = 0; i < NUM_PIXELS;i++){
        for(int j = 0;j < NUM_PIXELS;j++){
            img_sc_background[i][j] = img_sc[i][j];
            img_gsc_background[i][j] = img_gsc[i][j];
        }
    }
    background_collected = true;
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if (arg1 == 0){
        background_subtract = false;
    }
    if (arg1 == 2){
        background_subtract = true;
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    colorMap_sc->data()->fill(0);
    colorMap_gsc->data()->fill(0);
    colorMap_pa->data()->fill(0);
    colorMap_sc_gsc->data()->fill(0);

    memset(img_count,0,sizeof(img_count));
    memset(img_sc,0,sizeof(img_sc));
    memset(img_gsc,0,sizeof(img_gsc));
    memset(img_pa,0,sizeof(img_pa));

    memset(imgAcc_sc,0,sizeof(img_sc));
    memset(imgAcc_gsc,0,sizeof(img_gsc));
    memset(imgAcc_pa,0,sizeof(img_pa));

    memset(img_sc_background,0,sizeof(img_sc));
}
