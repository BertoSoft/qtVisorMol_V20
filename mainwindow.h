#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void on_actionNuevo_archivo_triggered();

private:
    Ui::MainWindow *ui;

    void initUi();
    void initBarraElementos();
    void initMenu();
    void setModoEdicion(bool isEdicion);

    // Variables de Clase
    bool isModoEdicion = false;
};
#endif // MAINWINDOW_H
