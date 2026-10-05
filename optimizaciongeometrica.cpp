#include "optimizaciongeometrica.h"
#include "ui_optimizaciongeometrica.h"

OptimizacionGeometrica::OptimizacionGeometrica(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptimizacionGeometrica)
{
    ui->setupUi(this);
}

OptimizacionGeometrica::~OptimizacionGeometrica()
{
    delete ui;
}
