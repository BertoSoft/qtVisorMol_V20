#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lienzomolecula.h"

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;

    void initUi();
    void initBarraElementos();
    void initLienzo();
    void initMenu();
    void setModoEdicion(bool isEdicion);

    //Funciones Privadas Internas Qt
    bool eventFilter(QObject *obj, QEvent *ev);

    // Variables de Clase
    bool            isModoEdicion = false;
    LienzoMolecula  *lienzo;
};
#endif // MAINWINDOW_H
