#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "AlazarControlThread.h"
#include "qcustomplot.h"
#include "acquisitionConfig.h"
#include "dataProcessingThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupTimeDomainPlot(QCustomPlot *customPlot);
    void setupAvgSigPlot(QCustomPlot *customPlot);
    void setupSigPlot(QCustomPlot *customPlot);
    void setupScImgPlot(QCustomPlot *customPlot);
    void setupPaImgPlot(QCustomPlot *customPlot);
    void setupGScImgPlot(QCustomPlot *customPlot);
    void setupScGScImgPlot(QCustomPlot *customPlot);

    void setupSigVsMirrorPlot(QCustomPlot *customPlot);
    void setupScVsGscVsMirrorPlot(QCustomPlot *customPlot);

private slots:
    void updateAvgSig();
    void updateSig();
    void updateFocusSig();

    void continuousSaveComplete();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_checkBox_stateChanged(int arg1);
    void on_pushButton_6_clicked();

    void on_lineEdit_2_returnPressed();

private:
    Ui::MainWindow *ui;
    AlazarControlThread dataThread;
    dataProcessingThread processingThread;

    QVector<double> ch1;
    QVector<double> ch2;
    QVector<double> ch3;
    QVector<double> ch4;
    QVector<double> x;

    QVector< QVector<double> > avgSig;

    QVector<double> sig_m1;
    QVector<double> sig_m2;
    QVector<double> sig_sc;
    QVector<double> sig_gsc;
    QVector<double> sig_pa;
    QVector<double> y;

    double focusSig_nsc;

    QCPColorMap *colorMap_sc;
    QCPColorMap *colorMap_pa;
    QCPColorMap *colorMap_gsc;
    QCPColorMap *colorMap_sc_gsc;

    int last_y_index;
    double img_count[NUM_PIXELS][NUM_PIXELS];
    double img_sc[NUM_PIXELS][NUM_PIXELS];
    double img_pa[NUM_PIXELS][NUM_PIXELS];
    double img_gsc[NUM_PIXELS][NUM_PIXELS];
    double imgAcc_sc[NUM_PIXELS][NUM_PIXELS];
    double imgAcc_pa[NUM_PIXELS][NUM_PIXELS];
    double imgAcc_gsc[NUM_PIXELS][NUM_PIXELS];

    double img_sc_background[NUM_PIXELS][NUM_PIXELS];
    double img_gsc_background[NUM_PIXELS][NUM_PIXELS];

    bool first_time;

    bool background_collected;
    bool background_subtract;

    bool startup;
};
#endif // MAINWINDOW_H
