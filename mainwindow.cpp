#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){
    ui->setupUi(this);

    initUi();
    setModoEdicion(false);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::initUi(){
    initBarraElementos();
    initMenu();
}

void MainWindow::initBarraElementos(){
    QActionGroup *actionGrupo = new QActionGroup(this);

    actionGrupo->addAction(ui->actionCarbono);
    actionGrupo->addAction(ui->actionHidrogeno);
    actionGrupo->addAction(ui->actionOxigeno);
    actionGrupo->addAction(ui->actionNitrogeno);

    actionGrupo->setObjectName("grupoElementos");
    actionGrupo->setExclusive(true);
}

void MainWindow::setModoEdicion(bool isEdicion){
    QActionGroup *actionGrupo = this->findChild<QActionGroup*>("grupoElementos");

    if(isEdicion == false){
        if(actionGrupo){
            actionGrupo->setExclusive(false);
        }
        for(QAction *action: ui->toolbarElementos->actions()){
            action->setChecked(false);
        }
    }
    else{
        if(actionGrupo){
            actionGrupo->setExclusive(true);
        }
        ui->actionCarbono->setChecked(true);
    }

    ui->toolbarElementos->setEnabled(isEdicion);
    ui->btnExportar->setEnabled(isEdicion);
    ui->btnLimpiar->setEnabled(isEdicion);
    ui->txtArgumentos->setEnabled(isEdicion);

    // Actualizamos la variable de clase
    isModoEdicion = isEdicion;
}

void MainWindow::initMenu(){
    ui->actionCerrar_archivo->setEnabled(isModoEdicion);
}


void MainWindow::on_actionNuevo_archivo_triggered(){
    setModoEdicion(true);
    initMenu();
}

