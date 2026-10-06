#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optimizaciongeometrica.h"

#include <QActionGroup>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QDate>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess>
#include <QDialog>
#include <QProgressDialog>
#include <QTextEdit>
#include <QPushButton>


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
    actualizarEstadosDeInterfaz();
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

    // Posicionamos la barra principal de Archivo en la zona superior
    addToolBar(Qt::TopToolBarArea, ui->toolbarMenu);

    // Posicionamos la barra de construcción y rotación a la izquierda (Vertical)
    addToolBar(Qt::LeftToolBarArea, ui->toolbarElementos);


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
    isModoEdicion = isEdicion;
    actualizarEstadosDeInterfaz();
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

void MainWindow::actualizarEstadosDeInterfaz(){
    // 1. Gestiones del Menú Archivo basadas en si estamos en modo edición o no
    ui->actionNuevo_archivo->setEnabled(!isModoEdicion);
    ui->actionAbrir_archivo->setEnabled(!isModoEdicion);
    ui->actionCerrar_archivo->setEnabled(isModoEdicion);
    ui->actionGuardar_archivo->setEnabled(isModoEdicion && isModificado);

    // 2. CORREGIDO: Desbloqueamos el visor gráfico para que responda a los clics del ratón
    ui->visorMolecular->setEnabled(isModoEdicion);
    ui->toolbarElementos->setEnabled(isModoEdicion);

    // 3. Menú de Cálculos (MOPAC)
    bool tieneAtomos = (lienzo->items().size() > 0);
    ui->actionOptimizaci_n_Geom_trica->setEnabled(isModoEdicion && tieneAtomos);

    // 4. Si la edición está apagada, limpiamos los checks de seguridad.
    if (!isModoEdicion) {
        ui->actionCarbono->setChecked(false);
        ui->actionHidrogeno->setChecked(false);
        ui->actionOxigeno->setChecked(false);
        ui->actionNitrogeno->setChecked(false);
        ui->actionModoEnlace->setChecked(false);
        ui->actionRotar_Molecula->setChecked(false);
    }
}


//###########################################################################################
// SLOTS privados
//#############################################################################################
void MainWindow::on_actionNuevo_archivo_triggered(){
    lienzo->limpiarLienzo();

    // CORREGIDO: Forzamos la activación del modo de edición usando la función propia
    setModoEdicion(true);

    txtTexto->setText("Archivo nuevo sin guardar...");
    rutaArchivoActual = "";
    isModificado = false;

    // Marcamos el Carbono por defecto y notificamos al lienzo
    ui->actionCarbono->setChecked(true);
    setNuevoElemento();

    actualizarTituloVentana();
}

void MainWindow::on_actionSalir_triggered(){
    this->close();
}

void MainWindow::on_actionCerrar_archivo_triggered(){
    lienzo->limpiarLienzo();
    setModoEdicion(false);
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
    // 1. Apagamos el modo rotación
    ui->actionRotar_Molecula->setChecked(false);
    lienzo->setModoRotar(false);

    // 2. Desmarcamos el átomo activo apagando momentáneamente la exclusividad
    QActionGroup *actionGrupo = this->findChild<QActionGroup*>("grupoElementos");
    if (actionGrupo) {
        actionGrupo->setExclusive(false);
        ui->actionCarbono->setChecked(false);
        ui->actionHidrogeno->setChecked(false);
        ui->actionOxigeno->setChecked(false);
        ui->actionNitrogeno->setChecked(false);
        actionGrupo->setExclusive(true);  // Restauramos la exclusividad
    }

    // 3. Forzamos el estado activo de Crear Enlace
    ui->actionModoEnlace->setChecked(true);
    lienzo->setModoEnlace(true);

    actualizarEstadosDeInterfaz();
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
    actualizarEstadosDeInterfaz();

    // Actualizamos la barra de estado con el nombre del archivo real guardado
    QFileInfo info(rutaArchivoActual);
    txtTexto->setText(" Archivo Activo: " + info.fileName());
}

void MainWindow::setNuevoElemento(){
    // 1. Apagamos visualmente los modos alternativos
    ui->actionModoEnlace->setChecked(false);
    ui->actionRotar_Molecula->setChecked(false);

    // 2. Apagamos de forma real sus estados lógicos en el lienzo
    lienzo->setModoEnlace(false);
    lienzo->setModoRotar(false); // Ahora sí guardará 'false' correctamente

    // 3. Asignamos el símbolo químico correspondiente al botón que esté activo
    if (ui->actionCarbono->isChecked())   lienzo->setElementoActual("C");
    else if (ui->actionHidrogeno->isChecked()) lienzo->setElementoActual("H");
    else if (ui->actionOxigeno->isChecked())   lienzo->setElementoActual("O");
    else if (ui->actionNitrogeno->isChecked()) lienzo->setElementoActual("N");

    setModoEdicion(true);
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
    // (Justo debajo de la zona ==== SOLUCIÓN DE REINICIOS Y MENÚS ====)
    isModificado = false;

    // CORREGIDO: Activamos el modo de edición de forma explícita
    setModoEdicion(true);

    lienzo->setModoEnlace(false);
    ui->actionModoEnlace->setChecked(false);

    // Refrescamos el título superior de la ventana
    actualizarTituloVentana();

}

void MainWindow::on_actionRotar_Molecula_triggered(){
    // 1. Apagamos el modo enlace
    ui->actionModoEnlace->setChecked(false);
    lienzo->setModoEnlace(false);

    // 2. Desmarcamos los elementos químicos manteniendo el grupo intacto
    QActionGroup *actionGrupo = this->findChild<QActionGroup*>("grupoElementos");
    if (actionGrupo) {
        actionGrupo->setExclusive(false);
        ui->actionCarbono->setChecked(false);
        ui->actionHidrogeno->setChecked(false);
        ui->actionOxigeno->setChecked(false);
        ui->actionNitrogeno->setChecked(false);
        actionGrupo->setExclusive(true);
    }

    // 3. Activamos el modo rotar en la interfaz y en el lienzo
    ui->actionRotar_Molecula->setChecked(true);
    lienzo->setModoRotar(true);

    actualizarEstadosDeInterfaz();
}

void MainWindow::on_actionOptimizaci_n_Geom_trica_triggered(){
    // 1. Abrimos el diálogo para configurar parámetros de MOPAC
    OptimizacionGeometrica dialogo(this);

    // Inicializamos sus combos y limpiamos basura estática del .ui antes de abrirlo
    dialogo.initUi();

    if (dialogo.exec() != QDialog::Accepted) {
        return; // Si el usuario cancela, salimos limpiamente
    }

    // 2. Extraemos los comandos configurados
    QString argumentosMopac = dialogo.getArgumentos();
    QString contenidoArchivo = lienzo->generarArchivoMOPAC(argumentosMopac);

    // 3. Escribimos el archivo temporal .mop
    QString rutaEntrada = QDir::tempPath() + "/calculo_temporal.mop";
    QFile archivoEntrada(rutaEntrada);
    if (!archivoEntrada.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "No se pudo crear el archivo de cálculo.");
        return;
    }
    QTextStream salida(&archivoEntrada);
    salida << contenidoArchivo;
    archivoEntrada.close();

    // 4. Creamos la barra de progreso flotante (UX Profesional)
    QProgressDialog progreso("Calculando optimización geométrica en MOPAC...", "Cancelar", 0, 0, this);
    progreso.setWindowTitle("Procesando");
    progreso.setWindowModality(Qt::WindowModal);
    progreso.show(); // Se muestra inmediatamente

    // 5. Actualizamos la barra de estado nativa antes de congelar
    txtTexto->setStyleSheet("QLabel { color: #3b82f6; background-color: #252525; }"); // Texto azul informativo
    txtTexto->setText(" ⏳ MOPAC está calculando la nueva estructura molecular...");
    this->setCursor(Qt::WaitCursor);

    // 6. Lanzamos MOPAC en segundo plano
    QProcess procesoMopac;
    procesoMopac.start("mopac", QStringList() << rutaEntrada);

    // Bucle inteligente: Espera a que MOPAC termine pero mantiene la interfaz despierta
    while (!procesoMopac.waitForFinished(100)) {
        QCoreApplication::processEvents(); // Evita que Windows diga "No responde"

        if (progreso.wasCanceled()) {
            procesoMopac.kill(); // Forzamos el cierre del ejecutable de MOPAC
            this->setCursor(Qt::ArrowCursor);
            txtTexto->setStyleSheet("QLabel { color: #ff0000; background-color: #252525; }");
            txtTexto->setText(" ❌ Cálculo cancelado por el usuario.");
            return;
        }
    }

    // 7. Restauramos el estado visual de la aplicación tras terminar MOPAC
    this->setCursor(Qt::ArrowCursor);
    progreso.close();

    // =========================================================================
    // VALIDACIÓN DE SISTEMA: ¿Respondió bien el ejecutable?
    // =========================================================================
    if (procesoMopac.exitStatus() != QProcess::NormalExit || procesoMopac.exitCode() != 0) {
        txtTexto->setStyleSheet("QLabel { color: #ff0000; background-color: #252525; }");
        txtTexto->setText(" ❌ Error crítico: El motor MOPAC se cerró de forma inesperada o no está instalado.");
        QMessageBox::critical(this, "Error de Sistema", "El ejecutable de MOPAC falló o devolvió un código de error.");
        return;
    }

    // 8. Leemos el archivo de salida físico (.out) generado por MOPAC
    QString rutaSalida = QDir::tempPath() + "/calculo_temporal.out";
    QFile archivoSalida(rutaSalida);
    if (!archivoSalida.open(QIODevice::ReadOnly | QIODevice::Text)) {
        txtTexto->setStyleSheet("QLabel { color: #ff0000; background-color: #252525; }");
        txtTexto->setText(" ❌ Error: No se encontró el archivo de salida (.out).");
        QMessageBox::critical(this, "Error de Archivo", "MOPAC terminó pero no se hallaron los resultados en el disco.");
        return;
    }

    QString contenidoSalida = archivoSalida.readAll();

    // GUARDAR COPIA PARA EL USUARIO
    ultimoResultadoMopac = contenidoSalida; // <-- AÑADE ESTA LÍNEA AQUÍ

    archivoSalida.close();

    // =========================================================================
    // VALIDACIÓN QUÍMICA: ¿El cálculo convergió matemáticamente?
    // =========================================================================
    if (!contenidoSalida.contains("JOB ENDED NORMALLY")) {
        txtTexto->setStyleSheet("QLabel { color: #ff0000; background-color: #252525; }");
        txtTexto->setText(" ❌ Error: El cálculo de MOPAC falló en la convergencia química.");

        // Buscamos pistas adicionales para avisar al usuario si fue culpa del SCF
        if (contenidoSalida.contains("FAILED") || contenidoSalida.contains("SCF FIELD")) {
            QMessageBox::warning(this, "MOPAC: Fallo de SCF", "La optimización falló. Comprueba que el espín y la carga asignados sean válidos para la cantidad de átomos de tu molécula.");
        } else {
            QMessageBox::warning(this, "MOPAC: Error Geométrico", "El motor MOPAC no pudo resolver la estructura con los parámetros asignados.");
        }
        return; // Salimos de forma segura SIN alterar ni romper los átomos del lienzo
    }

    // 9. Inyectamos la nueva geometría en tu lienzo si pasó todas las pruebas
    bool parseoOk = lienzo->actualizarGeometriaDesdeMOPACPOut(contenidoSalida);

    if (parseoOk) {
        isModificado = true;
        actualizarTituloVentana();
        txtTexto->setStyleSheet("QLabel { color: #00ff00; background-color: #252525; }"); // Texto verde éxito
        txtTexto->setText("  ¡Estructura molecular optimizada correctamente por MOPAC!");
    } else {
        txtTexto->setStyleSheet("QLabel { color: #ff0000; background-color: #252525; }");
        txtTexto->setText(" ❌ Error interno al parsear la geometría.");
        QMessageBox::warning(this, "Error Gráfico", "MOPAC calculó bien la molécula, pero el visor molecular no pudo leer las coordenadas resultantes.");
    }
}

void MainWindow::on_actionExportar_Archivo_triggered(){
    // 1. Validar que la molécula no esté vacía antes de exportar
    if (lienzo->items().isEmpty()) {
        QMessageBox::warning(this, "Exportar MOPAC", "No hay átomos en el lienzo para exportar.");
        return;
    }

    // 2. Configurar el diálogo para guardar el archivo físico
    QFileDialog dialogo(this, "Exportar Entrada MOPAC", QDir::currentPath());
    dialogo.setAcceptMode(QFileDialog::AcceptSave);
    dialogo.setNameFilter("Archivo de entrada MOPAC (*.mop);;Todos los archivos (*.*)");
    dialogo.setDefaultSuffix("mop"); // Si el usuario escribe "metano", se guarda como "metano.mop"

    if (dialogo.exec() != QDialog::Accepted) {
        return; // El usuario canceló el diálogo
    }

    // 3. Recuperar la ruta seleccionada
    QString rutaExportar = dialogo.selectedFiles().first();

    // 4. Intentar abrir el archivo en modo escritura de texto
    QFile archivo(rutaExportar);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error de Archivo", "No se pudo crear o escribir en el archivo seleccionado.");
        return;
    }

    // 5. Obtener el texto del lienzo e inyectarlo en el archivo físico
    // Usamos "PM7 EF SINGLET GEO-OK" para evitar errores por la planaridad inicial (Z=0)
    QString contenidoMopac = lienzo->generarArchivoMOPAC("PM7 EF SINGLET GEO-OK");

    QTextStream salida(&archivo);
    salida << contenidoMopac;
    archivo.close();

    // 6. Notificar el éxito en la interfaz
    QFileInfo info(rutaExportar);
    txtTexto->setStyleSheet("QLabel { color: #00ff00; background-color: #252525; }"); // Éxito en verde
    txtTexto->setText(" 💾 Archivo MOPAC exportado con éxito: " + info.fileName());
}

void MainWindow::on_actionVer_S_lida_MOPAC_triggered(){
    if (ultimoResultadoMopac.isEmpty()) {
        QMessageBox::information(this, "Salida de MOPAC", "Aún no se ha realizado ninguna optimización geométrica en esta sesión.");
        return;
    }

    // Crear la ventana emergente de forma dinámica
    QDialog *ventanaLog = new QDialog(this);
    ventanaLog->setWindowTitle("Visor de Resultados MOPAC (.out)");
    ventanaLog->resize(1000, 600); // Tamaño cómodo para leer tablas

    // Layout para organizar los componentes
    QVBoxLayout *layout = new QVBoxLayout(ventanaLog);

    // Caja de texto enriquecido/plano
    QTextEdit *txtConsola = new QTextEdit(ventanaLog);
    txtConsola->setReadOnly(true);
    txtConsola->setPlainText(ultimoResultadoMopac);

    // Aplicar tipografía monoespaciada (tipo Courier/Consolas) para que las tablas no se deformen
    QFont fuenteConsola("Monospace");
    fuenteConsola.setStyleHint(QFont::TypeWriter);
    fuenteConsola.setPointSize(14);
    txtConsola->setFont(fuenteConsola);

    // Botón de cierre
    QPushButton *btnCerrar = new QPushButton("Cerrar Visor", ventanaLog);
    connect(btnCerrar, &QPushButton::clicked, ventanaLog, &QDialog::accept);

    // Añadir componentes al layout
    layout->addWidget(txtConsola);
    layout->addWidget(btnCerrar);

    // Mostrar de forma modal (bloquea la ventana principal hasta que se cierre)
    ventanaLog->exec();

    // Liberar la memoria de la ventana al cerrarse
    delete ventanaLog;
}

