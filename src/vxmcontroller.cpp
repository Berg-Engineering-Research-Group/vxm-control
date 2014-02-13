#include "vxmcontroller.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

VXMController::VXMController(QObject *parent) :
    QObject(parent)
{
   serialConnection = new QSerialPort();
   isConnected = false;

   batchMovement = QByteArray();

   connect(serialConnection, SIGNAL(readyRead()), this, SLOT(serialReadyReadSlot()));
}

void VXMController::openSerialConnection(SerialSettings s) {
    serialConnection->setPortName(s.portName);
    if (serialConnection->open(QIODevice::ReadWrite)) {
        if (serialConnection->setBaudRate(s.baudRate)
                && serialConnection->setDataBits(s.dataBits)
                && serialConnection->setParity(s.parity)
                && serialConnection->setStopBits(s.stopBits)
                && serialConnection->setFlowControl(s.flowControl)) {

            isConnected = true;
            emit serialConnected();
            if (loggedWrite("F V\n") == -1)
                QMessageBox::critical(0, "Error", "Could not write to serial port");
        }
    }
}

void VXMController::closeSerialConnection() {
    if (serialConnection->isOpen()) {
        serialConnection->close();
    }
    isConnected = false;
    emit serialDisconnected();
}

bool VXMController::isSerialOpen() {
    return isConnected;
}

void VXMController::move(Direction d, int units) {
    emit serialBusy();

    QByteArray data = generateCommand(d, units);
    data.append(",R\n");
    if (loggedWrite(data) == -1) {
        QMessageBox::critical(0, "Error", "Could not write to serial port");
    }
}

void VXMController::batchMoveNew() {
    batchMovement = QByteArray("F ");
}

void VXMController::batchMoveAddMovement(Direction d, int units) {
    batchMovement.append(generateCommand(d, units));
    batchMovement.append(",");
}

void VXMController::batchMoveExec() {
    batchMovement.append("R");
    emit serialBusy();
    if (loggedWrite(batchMovement) == -1) {
        QMessageBox::critical(0,"Error", "Could not write to serial port");
    }
}

void VXMController::serialReadyReadSlot() {
    QByteArray data("");
    while (!serialConnection->atEnd()) {
        data.append(serialConnection->read(20));
    }
    // log read data
    QFile logFile("serialLog.txt");
    if (logFile.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream log(&logFile);
        log << " IN: " << data << endl;
        logFile.close();
    }

    if (data == "R")
        emit serialReady();

    if (data == "^") {
        loggedWrite("C\n");
        emit serialReady();
    }
}

int VXMController::loggedWrite(QByteArray data) {
    // write data to log file
    QFile logFile("serialLog.txt");
    if (logFile.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream log(&logFile);
        log << "OUT: " << data << endl;
        logFile.close();
    }
    // write data to serial port
    return serialConnection->write(data);
}

QByteArray VXMController::generateCommand(Direction d, int units) {
    QByteArray r;

    // switch direction if asked to move a negative number of units
    if (units < 0) {
        units *= -1;
        if      (d == MOVE_UP)    d = MOVE_DOWN;
        else if (d == MOVE_DOWN)  d = MOVE_UP;
        else if (d == MOVE_RIGHT) d = MOVE_LEFT;
        else if (d == MOVE_LEFT)  d = MOVE_RIGHT;
    }

    QByteArray steps;
    switch (d) {
        case MOVE_UP:
            r.append("I1M");
            steps = QByteArray::number(units);
            break;
        case MOVE_DOWN:
            r.append("I1M-");
            steps = QByteArray::number(units);
            break;
        case MOVE_LEFT:
            r.append("I2M-");
            steps = QByteArray::number(units);
            break;
        case MOVE_RIGHT:
            r.append("I2M");
            steps = QByteArray::number(units);
            break;
    }

    while (steps.length() < 3) {
        steps.prepend('0');
    }
    r.append(steps);
    return r;
}
