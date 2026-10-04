#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QGraphicsView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){
    ui->setupUi(this);

    initUi();
    setModoEdicion(false);

    ui->txtArgumentos->installEventFilter(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

//#########################################################################
// Funciones Protegidas
//##########################################################################
bool MainWindow::eventFilter(QObject *obj, QEvent *ev){

    // 1.- txtArgumentos -> FocusIn
    if(obj == ui->txtArgumentos && ev->type() == QEvent::FocusIn){
        if (ui->txtArgumentos->isEnabled()) {
            //Usando Qt::QueuedConnection, le ordenamos a Qt:
            //"Espera a que termines de procesar el clic del ratón, y
            //justo después de eso, selecciona todo el texto".
            QMetaObject::invokeMethod(ui->txtArgumentos, "selectAll", Qt::QueuedConnection);
        }
    }

    return QMainWindow::eventFilter(obj, ev);
}

//#########################################################################################
// Funciones Init
//######################################################################################
void MainWindow::initUi(){
    initLienzo();
    initBarraElementos();
    initMenu();
}

void MainWindow::initLienzo(){
    lienzo = new LienzoMolecula(this);
    ui->visorMolecular->setScene(lienzo);
}

void MainWindow::initBarraElementos(){

    // 1.- Creamos un actionGrupo para los elementos
    QActionGroup *actionGrupo = new QActionGroup(this);

    actionGrupo->addAction(ui->actionCarbono);
    actionGrupo->addAction(ui->actionHidrogeno);
    actionGrupo->addAction(ui->actionOxigeno);
    actionGrupo->addAction(ui->actionNitrogeno);

    actionGrupo->setObjectName("grupoElementos");
    actionGrupo->setExclusive(true);

    // 2. Nos aseguramos de que ambas barras estén asignadas al área superior
    addToolBar(Qt::TopToolBarArea, ui->toolbarMenu);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, ui->toolbarElementos);
}

void MainWindow::setModoEdicion(bool isEdicion){
    QActionGroup *actionGrupo = this->findChild<QActionGroup*>("grupoElementos");

    if(isEdicion == false){
        // 1.- ActionGrupo
        if(actionGrupo){
            actionGrupo->setExclusive(false);
        }
        for(QAction *action: ui->toolbarElementos->actions()){
            action->setChecked(false);
        }

        // 2.- txtArgumento
        ui->txtArgumentos->setText("PM7 PRECISE");
    }
    else{
        // 1.- ActionGrupo
        if(actionGrupo){
            actionGrupo->setExclusive(true);
        }
        ui->actionCarbono->setChecked(true);
    }

    ui->toolbarElementos->setEnabled(isEdicion);
    ui->btnExportar->setEnabled(isEdicion);
    ui->btnLimpiar->setEnabled(isEdicion);
    ui->txtArgumentos->setEnabled(isEdicion);
    ui->visorMolecular->setEnabled(isEdicion);

    // Actualizamos la variable de clase
    isModoEdicion = isEdicion;
}

void MainWindow::initMenu(){
    ui->actionCerrar_archivo->setEnabled(isModoEdicion);
    ui->actionNuevo_archivo->setEnabled(!isModoEdicion);
}

//###########################################################################################
// SLOTS privados
//#############################################################################################
void MainWindow::on_actionNuevo_archivo_triggered(){
    setModoEdicion(true);
    initMenu();
}

void MainWindow::on_actionSalir_triggered(){
    exit(0);
}

void MainWindow::on_actionCerrar_archivo_triggered(){
    setModoEdicion(false);
    initMenu();
}

void MainWindow::on_actionCarbono_triggered(){
    setNuevoElemento();
}


void MainWindow::on_actionHidrogeno_triggered(){
    setNuevoElemento();
}


void MainWindow::on_actionOxigeno_triggered(){
    setNuevoElemento();
}


void MainWindow::on_actionNitrogeno_triggered(){
    setNuevoElemento();
}


void MainWindow::on_actionModoEnlace_triggered(){
    QActionGroup *actionGrupo = this->findChild<QActionGroup*>("grupoElementos");

    if(actionGrupo){
        for(QAction *action: ui->toolbarElementos->actions()){
            action->setChecked(false);
        }
    }
    ui->actionModoEnlace->setChecked(true);
    lienzo->setModoEnlace(ui->actionModoEnlace->isChecked());
}

void MainWindow::on_btnLimpiar_clicked(){
    lienzo->limpiarLienzo();
}


void MainWindow::setNuevoElemento(){
    ui->actionModoEnlace->setChecked(false);

    if (ui->actionCarbono->isChecked())   lienzo->setElementoActual("C");
    if (ui->actionHidrogeno->isChecked()) lienzo->setElementoActual("H");
    if (ui->actionOxigeno->isChecked())   lienzo->setElementoActual("O");
    if (ui->actionNitrogeno->isChecked()) lienzo->setElementoActual("N");
}


