#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VTKCuttingAndProbing.h"

#include <QVTKOpenGLNativeWidget.h>

class VTKCuttingAndProbing : public QMainWindow
{
    Q_OBJECT

public:
    VTKCuttingAndProbing(QWidget *parent = nullptr);
    ~VTKCuttingAndProbing();

private:
    Ui::VTKCuttingAndProbingClass ui;

    QVTKOpenGLNativeWidget* _pVTKWidget = Q_NULLPTR;
};
