#include "optimizaciongeometrica.h"
#include "ui_optimizaciongeometrica.h"

OptimizacionGeometrica::OptimizacionGeometrica(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptimizacionGeometrica){

    ui->setupUi(this);
}

OptimizacionGeometrica::~OptimizacionGeometrica(){
    delete ui;
}

QString OptimizacionGeometrica::getArgumentos(){
    return ui->etArgumentos->text();
}

void OptimizacionGeometrica::initUi(){

    initSp();
    initSpin();
    setPreviewArgumentos();
}

void OptimizacionGeometrica::initSpin(){
    ui->spinCarga->setMinimum(-4);
    ui->spinCarga->setMaximum(4);

    ui->spinCarga->setValue(0);
}

void OptimizacionGeometrica::initSp(){

    // 1. Limpiamos los elementos estáticos heredados del archivo .ui
    ui->cmbMetodo->clear();
    ui->cmbTarea->clear();
    ui->cmbMultiplicidad->clear();

    // Metodo
    ui->cmbMetodo->addItem("PM7 (Recomendado)", "PM7");
    ui->cmbMetodo->addItem("PM7-TS", "PM7-TS");
    ui->cmbMetodo->addItem("PM6", "PM6");
    ui->cmbMetodo->addItem("RM1", "RM1");
    ui->cmbMetodo->addItem("AM1", "AM1");

    ui->cmbMetodo->insertSeparator(5);

    ui->cmbMetodo->addItem("MNDO", "MNDO");
    ui->cmbMetodo->addItem("PM3", "PM3");

    // Tipo de calculo
    ui->cmbTarea->addItem("EF (Optimización Geométrica - Estándar)", "EF");
    ui->cmbTarea->addItem("FORCE (Cálculo de Frecuencias e IR)", "FORCE");
    ui->cmbTarea->addItem("1SCF (Energía de Punto Único)", "1SCF");
    ui->cmbTarea->addItem("TS (Búsqueda de Estado de Transición)", "TS");

    ui->cmbTarea->insertSeparator(4);

    ui->cmbTarea->addItem("BFGS (Optimización Alternativa)", "BFGS");
    ui->cmbTarea->addItem("IRC (Coordenada de Reacción)", "IRC");
    ui->cmbTarea->addItem("DRC (Dinámica Molecular)", "DRC");
    ui->cmbTarea->addItem("POLAR (Polarizabilidad)", "POLAR");

    // Modo del estado
    ui->cmbMultiplicidad->addItem("SINGLET (Singlete - Capa Cerrada)", "SINGLET");
    ui->cmbMultiplicidad->addItem("DOUBLET (Doblete - Radicales / Impar)", "DOUBLET");
    ui->cmbMultiplicidad->addItem("TRIPLET (Triplete - O2 / Excitados)", "TRIPLET");

    ui->cmbMultiplicidad->insertSeparator(3);

    ui->cmbMultiplicidad->addItem("QUARTET (Cuartete)", "QUARTET");
    ui->cmbMultiplicidad->addItem("QUINTET (Quintete)", "QUINTET");
    ui->cmbMultiplicidad->addItem("SEXTET (Sextete)", "SEXTET");
    ui->cmbMultiplicidad->addItem("SEPTET (Septete)", "SEPTET");

}

void OptimizacionGeometrica::setPreviewArgumentos(){
    // 1. Extraemos las Keywords reales guardadas en el UserData de los combos
    QString metodo        = ui->cmbMetodo->currentData().toString();
    QString tarea         = ui->cmbTarea->currentData().toString();
    QString multiplicidad = ui->cmbMultiplicidad->currentData().toString();

    // SEGURIDAD: Si cualquiera de los datos está vacío (porque initUi aún está rellenándolos),
    // salimos de la función inmediatamente para evitar fallos.
    if (metodo.isEmpty() || tarea.isEmpty() || multiplicidad.isEmpty()) {
        return;
    }

    // 2. Leemos el SpinBox de la carga
    int valorCarga = ui->spinCarga->value();

    // 3. Empezamos a concatenar los comandos básicos obligatorios
    QString comandoFinal = QString("%1 %2 %3").arg(metodo, tarea, multiplicidad);

    // 4. MOPAC solo requiere la palabra clave CHARGE si el valor es distinto de cero
    if (valorCarga != 0) {
        comandoFinal += QString(" CHARGE=%1").arg(valorCarga);
    }

    // 5. Agregamos el modificador estricto de precisión si el usuario marcó el CheckBox
    if (ui->chkPrecision->isChecked()) {
        comandoFinal += " PRECISE";
    }

    // 6. Inyectamos la cadena limpia dentro de la barra verde fosforito (etArgumentos)
    ui->etArgumentos->setText(comandoFinal.trimmed().toUpper());
}

void OptimizacionGeometrica::on_cmbMetodo_activated(int index){
    setPreviewArgumentos();
}


void OptimizacionGeometrica::on_cmbTarea_activated(int index){
    setPreviewArgumentos();
}


void OptimizacionGeometrica::on_spinCarga_valueChanged(int arg1){
    setPreviewArgumentos();
}


void OptimizacionGeometrica::on_cmbMultiplicidad_activated(int index){
    setPreviewArgumentos();
}


void OptimizacionGeometrica::on_chkPrecision_checkStateChanged(const Qt::CheckState &arg1){
    setPreviewArgumentos();
}


void OptimizacionGeometrica::on_buttonBox_accepted(){
    this->accept();
}


void OptimizacionGeometrica::on_buttonBox_rejected(){
    this->reject();
}

