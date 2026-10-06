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

    QString getArgumentos();
    void    initUi();


private slots:
    void on_cmbMetodo_activated(int index);

    void on_cmbTarea_activated(int index);

    void on_spinCarga_valueChanged(int arg1);

    void on_cmbMultiplicidad_activated(int index);

    void on_chkPrecision_checkStateChanged(const Qt::CheckState &arg1);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::OptimizacionGeometrica *ui;

    void initSpin();
    void initSp();
    void setPreviewArgumentos();
};

#endif // OPTIMIZACIONGEOMETRICA_H
