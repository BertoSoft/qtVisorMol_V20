#ifndef LIENZOMOLECULA_H
#define LIENZOMOLECULA_H



#include <QGraphicsScene>
#include <QVector>
#include <QString>

// =========================================================================
// ESTRUCTURAS DE DATOS ORIGINALES
// =========================================================================

// Representa la información química y la ubicación en pantalla de un átomo
struct Atomo {
    int     id;        // Identificador único y numérico para cada átomo (0, 1, 2...)
    QString simbolo;    // Símbolo químico del elemento (Ej: "C", "H", "O", "N")
    QPointF posicion;  // Coordenadas bidimensionales (X, Y) del átomo en el lienzo de dibujo
};

// Representa la unión covalente o enlace químico entre dos átomos
struct Enlace {
    int id_atomo1;     // ID del átomo donde inicia el enlace
    int id_atomo2;     // ID del átomo donde termina el enlace
};

// =========================================================================
// CLASE PRINCIPAL: MOLECULACANVAS
// =========================================================================
// Hereda de QGraphicsScene para poder gestionar un espacio de dibujo 2D interactivo
class LienzoMolecula : public QGraphicsScene {
    Q_OBJECT // Macro obligatoria de Qt para permitir el uso de señales y slots

public:
    // Constructor: Inicializa el lienzo y define el tamaño del área de dibujo
    explicit LienzoMolecula(QObject *parent = nullptr);

    // Destructor: Se encarga de liberar la memoria de forma segura al cerrar la aplicación
    ~LienzoMolecula() override;

    // Cambia el elemento químico activo que se dibujará al hacer clic (Ej: "C", "H")
    void setElementoActual(const QString& elemento);

    // Activa o desactiva el modo de creación de enlaces entre átomos existentes
    void setModoEnlace(bool activo);

    // Borra por completo todos los átomos, enlaces y gráficos del lienzo
    void limpiarLienzo();

    // Transforma los datos lógicos de la molécula a código de coordenadas cartesianas MOPAC (.mop)
    QString generarContenidoMOPAC(const QString& argumentos) const;

protected:
    // Evento nativo de Qt que captura automáticamente cuando el usuario hace clic con el ratón en el lienzo
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEv) override;

private:
    // =========================================================================
    // CONTENEDORES DE DATOS (ESTRUCTURA LÓGICA DE LA MOLÉCULA)
    // =========================================================================
    QVector<Atomo>  listaAtomos;    // Lista dinámica que almacena todos los átomos creados
    QVector<Enlace> listaEnlaces;   // Lista dinámica que almacena todas las uniones entre átomos

    // =========================================================================
    // VARIABLES DE ESTADO Y CONTROL DEL LIENZO
    // =========================================================================
    QString elementoActual          = "C";      // Almacena el texto del átomo activo seleccionado en la interfaz ("C", "H", etc.)
    bool    modoEnlaceActivo        = false;    // Controla el comportamiento del clic: true (une átomos), false (crea átomos)
    int     atomoSeleccionadoId     = -1;       // Guarda temporalmente el ID del primer átomo seleccionado al trazar un enlace (-1 si no hay ninguno)
    int     contadorIds             = 0;        // Generador secuencial automático para asignar IDs únicos a los átomos creados

    // =========================================================================
    // FUNCIONES AUXILIARES PRIVADAS
    // =========================================================================

    // Escanea el lienzo para verificar si un clic ocurrió cerca o sobre un átomo existente.
    // Retorna el índice del QVector donde está guardado el átomo, o -1 si hizo clic en el vacío.
    int     buscarAtomoEnPosicion(const QPointF &pos) const;

    // Devuelve el color oficial CPK (estándar químico) correspondiente al elemento químico para pintarlo visualmente
    QColor  getColorElemento(const QString &elemento) const;

    // Devuelve el color Radio correspondiente al elemento químico para pintarlo visualmente
    int  getRadioElemento(const QString &elemento) const;
};

#endif // LIENZOMOLECULA_H
