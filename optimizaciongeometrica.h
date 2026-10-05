#ifndef OPTIMIZACIONGEOMETRICA_H
#define OPTIMIZACIONGEOMETRICA_H

#include <QDialog>

namespace Ui {
class OptimizacionGeometrica;
}

class OptimizacionGeometrica : public QDialog
{
    Q_OBJECT

public:
    explicit OptimizacionGeometrica(QWidget *parent = nullptr);
    ~OptimizacionGeometrica();

private:
    Ui::OptimizacionGeometrica *ui;
};

#endif // OPTIMIZACIONGEOMETRICA_H
