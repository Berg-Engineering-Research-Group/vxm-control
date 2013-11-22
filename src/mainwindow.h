#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialconfigdialog.h"
#include "vxmcontroller.h"
#include <QMainWindow>
#include <QLabel>
#include <QCamera>
#include <QCameraViewfinder>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum {DRAW_MANUAL, DRAW_LINE, DRAW_RECT};
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionSerialConfig_triggered();
    void on_btnConnect_clicked();
    void on_btnDoMove_clicked();
    void on_btnGrpDrawType_buttonClicked(int);

    void controller_connected();
    void controller_disconnected();
    void controller_ready();
    void controller_busy();

    void camera_error(QCamera::Error);

private:
    void toggleManualControls(bool);
    void refreshMoveBtnState();

    Ui::MainWindow *ui;
    SerialConfigDialog *serialDialog;

    QCamera *camera;
    QCameraViewfinder *viewFinder;

    QLabel *labelConnectionStatus;
    VXMController *controller;

    QPoint shapeStart;
    QPoint shapeEnd;
    bool shapeDrawn;
};

#endif
