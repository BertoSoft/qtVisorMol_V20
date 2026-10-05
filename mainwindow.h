#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lienzomolecula.h"

#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;



    // VAriables y Ctes de clase
    QString NAME_APP ="Visor Molecular V2.0";

private slots:

    void setNuevoElemento();

    void on_actionNuevo_archivo_triggered();

    void on_actionSalir_triggered();

    void on_actionCerrar_archivo_triggered();

    void on_actionCarbono_triggered();

    void on_actionHidrogeno_triggered();

    void on_actionOxigeno_triggered();

    void on_actionNitrogeno_triggered();

    void on_actionModoEnlace_triggered();

    void on_btnLimpiar_clicked();

    void on_actionGuardar_archivo_triggered();

    void on_actionAbrir_archivo_triggered();

    void on_actionRotar_Molecula_triggered();

protected:
    void closeEvent(QCloseEvent *ev) override;

private:
    Ui::MainWindow *ui;

    void initUi();
    void initBarraElementos();
    void initLienzo();
    void initMenu();
    void initBarraEstado();
    void setModoEdicion(bool isEdicion);
    void actualizarTituloVentana();

    //Funciones Privadas Internas Qt
    bool eventFilter(QObject *obj, QEvent *ev);

    // Variables de Clase
    bool            isModoEdicion = false;
    bool            isModificado = false;
    LienzoMolecula  *lienzo;
    QString         rutaArchivoActual;

    // Componentes de la barra de estado
    QStatusBar      *barraEstado;
    QLabel          *txtTexto;
    QLabel          *txtFecha;
    QLabel          *txtHora;
};
#endif // MAINWINDOW_H
