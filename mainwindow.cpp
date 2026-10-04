#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QDate>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>

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

//#########################################################################
// Funciones Protegidas
//##########################################################################
bool MainWindow::eventFilter(QObject *obj, QEvent *ev){



    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::closeEvent(QCloseEvent *ev){
    if(isModificado){
        QMessageBox::StandardButton respuesta;
        respuesta = QMessageBox::question(
            this,
            "Cambios sin Guardar",
            "La molécula actual tiene modificaciones pendientes. ¿Deseas guardar los cambios antes de salir?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        // Procesamos la respuesta
        if(respuesta == QMessageBox::Save){
            on_actionGuardar_archivo_triggered();
            ev->accept();
        }
        else if(respuesta == QMessageBox::Discard){
            ev->accept();
        }
        else{
            ev->ignore();
        }
    }
    else{
        ev->accept();
    }
}

//#########################################################################################
// Funciones Init
//######################################################################################
void MainWindow::initUi(){
    setWindowTitle(NAME_APP);

    initLienzo();
    initBarraElementos();
    initBarraEstado();
    initMenu();
}

void MainWindow::initLienzo(){
    lienzo = new LienzoMolecula(this);
    ui->visorMolecular->setScene(lienzo);

    connect(lienzo, &LienzoMolecula::contenidoModificado, this, [this](){
        isModificado = true;
        actualizarTituloVentana();
    });
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

void MainWindow::initBarraEstado(){
    barraEstado = new QStatusBar(this);
    setStatusBar(barraEstado);

    txtTexto    = new QLabel("", this);
    txtFecha    = new QLabel("", this);
    txtHora     = new QLabel("", this);

    // Aplicamos el estilo a los labels
    QList<QLabel*> listaTxt = {txtFecha, txtHora, txtTexto};
    for(QLabel *label: listaTxt){
        label->setFrameShape(QFrame::Panel);     // Define el marco tipo panel
        label->setFrameShadow(QFrame::Sunken);  // Aplica el efecto hundido/sombra interior
        label->setAlignment(Qt::AlignCenter);   // Centra el texto vertical y horizontalmente

        // Estilo estético opcional mediante CSS para suavizar los bordes internos
        label->setStyleSheet(
            "QLabel {"
            "   border-color: #333333;" // Color gris oscuro a juego con tu visor molecular
            "   padding: 2px 6px;"      // Margen interno para que el texto no toque los bordes hundidos
            "   background-color: #252525;" // Fondo ligeramente más claro que el del lienzo
            "   color: #e0e0e0;"        // Texto gris claro
            "}"
            );
    }
    txtTexto->setAlignment(Qt::AlignLeft);

    // Añadimos txtTexto a la izquierda de forma expandida
    // El argumento '1' indica el factor de estiramiento para ocupar el espacio disponible
    barraEstado->addWidget(txtTexto, 1);

    // Añadimos txtFecha y txtHora como widgets permanentes a la derecha
    barraEstado->addPermanentWidget(txtFecha);
    barraEstado->addPermanentWidget(txtHora);

    // Conectamos QTimer a los label
    txtFecha->setText(QDate::currentDate().toString("dddd, d 'de' MMMM 'de' yyyy"));
    txtHora->setText(QTime::currentTime().toString("hh:mm:ss"));

    QTimer *reloj = new QTimer(this);
    connect(reloj, &QTimer::timeout, this, [this](){
        txtFecha->setText(QDate::currentDate().toString("dddd, d 'de' MMMM 'de' yyyy"));
        txtHora->setText(QTime::currentTime().toString("hh:mm:ss"));
    });
    reloj->start(1000);

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
    }
    else{
        // 1.- ActionGrupo
        if(actionGrupo){
            actionGrupo->setExclusive(true);
        }
        ui->actionCarbono->setChecked(true);
    }

    ui->toolbarElementos->setEnabled(isEdicion);
    ui->visorMolecular->setEnabled(isEdicion);

    // Actualizamos la variable de clase
    isModoEdicion = isEdicion;
}

void MainWindow::initMenu(){
    ui->actionCerrar_archivo->setEnabled(isModoEdicion);
    ui->actionNuevo_archivo->setEnabled(!isModoEdicion);
}

//########################################################################################
// Funciones Privadas
//#########################################################################################
void MainWindow::actualizarTituloVentana(){
    QString nombreArchivo = "Proyecto sin guardar...";

    if(!rutaArchivoActual.isEmpty()){
        QFileInfo info(rutaArchivoActual);
        nombreArchivo = info.fileName();
    }

    // Si se ha modificado y no se guardo, añadimos el *
    if(isModificado && !rutaArchivoActual.isEmpty()){
        nombreArchivo += "*";
    }

    txtTexto->setText(nombreArchivo);
    setWindowTitle(nombreArchivo + " - " +NAME_APP);
}

//###########################################################################################
// SLOTS privados
//#############################################################################################
void MainWindow::on_actionNuevo_archivo_triggered(){
    lienzo->limpiarLienzo();
    setModoEdicion(true);
    initMenu();
    txtTexto->setText("Archivo nuevo sin guardar...");
    rutaArchivoActual = "";
    isModificado = false;
    actualizarTituloVentana();
}

void MainWindow::on_actionSalir_triggered(){
    this->close();
}

void MainWindow::on_actionCerrar_archivo_triggered(){
    lienzo->limpiarLienzo();
    setModoEdicion(false);
    initMenu();
    rutaArchivoActual = "";   // <--- Es buena idea asegurar limpiar la ruta aquí
    isModificado = false;     // <--- Y asegurar limpiar el estado

    actualizarTituloVentana(); // <--- AGREGAR AQUÍ
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

void MainWindow::on_actionGuardar_archivo_triggered(){
    if (rutaArchivoActual.isEmpty()) {
        QFileDialog dialogo(this, "Guardar Proyecto Molecular", QDir::currentPath());
        dialogo.setAcceptMode(QFileDialog::AcceptSave);
        dialogo.setNameFilter("Proyecto VisorMol (*.json)");

        // FORZADO DE SUFIJO: Si el usuario escribe "agua", Qt lo convierte en "agua.json"
        dialogo.setDefaultSuffix("json");

        if (dialogo.exec() != QDialog::Accepted) return;

        // Recuperamos la ruta validada por el diálogo
        rutaArchivoActual = dialogo.selectedFiles().first();
    }

    // Intentamos abrir el archivo
    QFile archivo(rutaArchivoActual);
    if(!archivo.open(QIODevice::WriteOnly | QIODevice::Text)){
        return;
    }

    QJsonObject     datosMolecula = lienzo->generarArchivoJson();
    QJsonDocument   documento(datosMolecula);

    // Grabamos los datos formateados con saltos de línea para que sea legible
    archivo.write(documento.toJson(QJsonDocument::Indented));
    archivo.close();

    isModificado = false;
    actualizarTituloVentana();

    // Actualizamos la barra de estado con el nombre del archivo real guardado
    QFileInfo info(rutaArchivoActual);
    txtTexto->setText(" Archivo Activo: " + info.fileName());
}

void MainWindow::setNuevoElemento(){
    ui->actionModoEnlace->setChecked(false);

    if (ui->actionCarbono->isChecked())   lienzo->setElementoActual("C");
    if (ui->actionHidrogeno->isChecked()) lienzo->setElementoActual("H");
    if (ui->actionOxigeno->isChecked())   lienzo->setElementoActual("O");
    if (ui->actionNitrogeno->isChecked()) lienzo->setElementoActual("N");

    setModoEdicion(true);
    lienzo->setModoEnlace(false);
}

void MainWindow::on_actionAbrir_archivo_triggered(){
    if(isModificado){
        QMessageBox::StandardButton respuesta;
        respuesta = QMessageBox::question(
            this,
            "Cambios sin Guardar",
            "La molécula actual tiene modificaciones pendientes. ¿Deseas guardar los cambios antes de seguir?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );
        // Procesamos la respuesta
        if(respuesta == QMessageBox::Save){
            on_actionGuardar_archivo_triggered();
        }
        else if(respuesta == QMessageBox::Cancel){
            return;
        }
    }

    QString ruta = QFileDialog::getOpenFileName(
        this,
        "Abrir archivo de datos VisorMol...",
        QDir::currentPath(),
        "Proyecto VisorMol (*.json);;Todos los Archivos (*.*)" // Nota: En Qt los filtros se separan con doble punto y coma (;;)
        );

    if(ruta.isEmpty()) return;

    QFile archivo(ruta);
    if(!archivo.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(this, "Error", "No se pudo abrir el archivo seleccionado.");
        return;
    }

    // ==== AÑADE ESTE BLOQUE DESDE AQUÍ ====
    // Leemos todo el contenido de texto del archivo físico
    QByteArray datosBytes = archivo.readAll();
    archivo.close();

    // Parseamos los bytes en un documento JSON estructurado
    QJsonDocument documentoJson = QJsonDocument::fromJson(datosBytes);
    if (documentoJson.isNull() || !documentoJson.isObject()) {
        QMessageBox::critical(this, "Error", "El archivo no contiene un formato JSON molecular válido.");
        return;
    }

    // Guardamos la ruta activa si el JSON es válido
    rutaArchivoActual = ruta;

    // Pasamos el objeto raíz del JSON a la nueva función del lienzo
    QJsonObject objetoRaiz = documentoJson.object();
    lienzo->cargarArchivoJson(objetoRaiz);
    // ======================================

    // ==== SOLUCIÓN DE REINICIOS Y MENÚS ====
    isModificado = false;

    // 1. Activamos el modo edición y dejamos el Carbono seleccionado por defecto
    setModoEdicion(true);
    lienzo->setModoEnlace(false); // Nos aseguramos de que empiece añadiendo átomos y no enlaces
    ui->actionModoEnlace->setChecked(false);

    // 2. Sincronizamos los estados de habilitación de la barra de menús
    initMenu();

    // 3. Refrescamos el título superior de la ventana
    actualizarTituloVentana();

}

