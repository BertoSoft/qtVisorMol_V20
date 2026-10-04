#include "lienzomolecula.h"

#include <QPointF>
#include <QString>
#include <QObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QLine>
#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>

//################################################################################################
// Funciones Publicas
//############################################################################################

LienzoMolecula::LienzoMolecula(QObject *parent): QGraphicsScene(parent) {

    setSceneRect(-1000, -1000, 2000, 2000);
}

LienzoMolecula::~LienzoMolecula(){
    limpiarLienzo();
}

void LienzoMolecula::setElementoActual(const QString &elemento){
    elementoActual = elemento;
}

void LienzoMolecula::setModoEnlace(bool isActivo){
    modoEnlaceActivo = isActivo;
    atomoSeleccionadoId = -1;
}

void LienzoMolecula::limpiarLienzo(){
    clear();
    listaAtomos.clear();
    listaEnlaces.clear();
    atomoSeleccionadoId = -1;
    contadorIds = 0;
}

QString LienzoMolecula::generarArchivoMOPAC(const QString &argumentos) const{
    // Primera y segunda línea: Argumentos y título obligatorio para MOPAC
    QString contenido = argumentos + "\n";
    contenido += "Molecula generada con VisorMol_V10\n\n";

    // Escribimos la tabla de coordenadas cartesianas (Matriz Z simplificada)
    for (const Atomo &atomo : listaAtomos) {
        // Formato estándar MOPAC: Simbolo X 1 Y 1 Z 1
        // (Los '1' le indican a MOPAC que optimice geométricamente esa coordenada)
        contenido += QString("%1   %2 1   %3 1   %4 1\n")
                         .arg(atomo.simbolo)
                         .arg(atomo.posicion.x() / 50.0, 0, 'f', 4)
                         .arg(atomo.posicion.y() / 50.0, 0, 'f', 4)
                         .arg(atomo.posicion.z() / 50.0, 0, 'f', 4); // <-- Z REAL
    }
    return contenido;
}

QJsonObject LienzoMolecula::generarArchivoJson() const{
    QJsonObject objetoRaiz;

    // Serializamos la estructura atomos
    QJsonArray arrayAtomos;
    for(const Atomo &atomo: listaAtomos){
        QJsonObject nodoAtomo;
        nodoAtomo["id"]         = atomo.id;
        nodoAtomo["simbolo"]    = atomo.simbolo;
        nodoAtomo["x"]          = atomo.posicion.x();
        nodoAtomo["y"]          = atomo.posicion.y();
        nodoAtomo["z"]          = atomo.posicion.z();
        arrayAtomos.append(nodoAtomo);
    }
    objetoRaiz["atomos"] = arrayAtomos;

    // Serializamos los enlaces
    QJsonArray arrayEnlaces;
    for(const Enlace &enlace: listaEnlaces){
        QJsonObject nodoEnlace;
        nodoEnlace["id_atomo1"] = enlace.id_atomo1;
        nodoEnlace["id_atomo2"] = enlace.id_atomo2;
        nodoEnlace["orden"] = enlace.orden;
        arrayEnlaces.append(nodoEnlace);
    }
    objetoRaiz["enlaces"] = arrayEnlaces;

    return objetoRaiz;
}

void LienzoMolecula::cargarArchivoJson(const QJsonObject &objetoRaiz){
    // 1. Limpiamos cualquier rastro de la molécula anterior
    limpiarLienzo();

    // 2. Reconstruimos los Átomos
    QJsonArray arrayAtomos = objetoRaiz["atomos"].toArray();
    int maxId = -1;

    for (int i = 0; i < arrayAtomos.size(); ++i) {
        QJsonObject nodoAtomo = arrayAtomos[i].toObject();
        Atomo atomo;
        atomo.id      = nodoAtomo["id"].toInt();
        atomo.simbolo = nodoAtomo["simbolo"].toString();

        // Extraemos las coordenadas espaciales
        double x = nodoAtomo["x"].toDouble();
        double y = nodoAtomo["y"].toDouble();
        double z = nodoAtomo["z"].toDouble();
        atomo.posicion = QVector3D(x, y, z);

        // Controlamos cuál es el ID más alto para no repetir secuencias al añadir nuevos elementos
        if (atomo.id > maxId) {
            maxId = atomo.id;
        }

        listaAtomos.append(atomo);
    }
    // El siguiente átomo nuevo continuará la secuencia a partir del más alto
    contadorIds = maxId + 1;

    // 3. Reconstruimos los Enlaces
    QJsonArray arrayEnlaces = objetoRaiz["enlaces"].toArray();
    for (int i = 0; i < arrayEnlaces.size(); ++i) {
        QJsonObject nodoEnlace = arrayEnlaces[i].toObject();
        Enlace enlace;
        enlace.id_atomo1 = nodoEnlace["id_atomo1"].toInt();
        enlace.id_atomo2 = nodoEnlace["id_atomo2"].toInt();
        enlace.orden     = nodoEnlace["orden"].toInt();

        listaEnlaces.append(enlace);
    }

    // 4. Forzamos a Qt a renderizar los nuevos elementos en la escena gráfica
    actualizarRenderizado();
}


//##########################################################################################
// Funciones Protegidas
//#####################################################################################

void LienzoMolecula::mousePressEvent(QGraphicsSceneMouseEvent *mouseEv){
    QPointF posClick = mouseEv->scenePos();
    int indiceAtomoClick = buscarAtomoEnPosicion(posClick);

    // MODO AÑADIR ÁTOMO
    if (!modoEnlaceActivo) {
        if (indiceAtomoClick == -1) {
            Atomo nuevoAtomo;
            nuevoAtomo.id       = contadorIds++;
            nuevoAtomo.simbolo  = elementoActual;
            nuevoAtomo.posicion = QVector3D(posClick.x(), posClick.y(), 0.0);

            listaAtomos.append(nuevoAtomo);
            actualizarRenderizado(); // Redibuja todo de forma limpia
            emit contenidoModificado();
        }
    }
    // MODO ENLACE
    else {
        if (indiceAtomoClick != -1) {
            int idAtomo1 = atomoSeleccionadoId;
            int idAtomo2 = listaAtomos[indiceAtomoClick].id;

            if (idAtomo1 == -1) {
                atomoSeleccionadoId = idAtomo2; // Primer átomo seleccionado
            }
            else {
                if (idAtomo2 != idAtomo1) {
                    // 1. Buscar si YA existe un enlace entre estos dos átomos
                    int indiceEnlaceExistente = -1;
                    for (int i = 0; i < listaEnlaces.size(); ++i) {
                        if ((listaEnlaces[i].id_atomo1 == idAtomo1 && listaEnlaces[i].id_atomo2 == idAtomo2) ||
                            (listaEnlaces[i].id_atomo1 == idAtomo2 && listaEnlaces[i].id_atomo2 == idAtomo1)) {
                            indiceEnlaceExistente = i;
                            break;
                        }
                    }

                    // 2. Si YA EXISTE, aumentamos el orden (Simple -> Doble -> Triple -> Eliminar)
                    if (indiceEnlaceExistente != -1) {
                        Enlace &enlace = listaEnlaces[indiceEnlaceExistente];

                        // VALIDACIÓN VALENCIAS (Para subir el orden del enlace)
                        //int valizq = getMaxValencia(idAtomo1); // Lo ideal es buscar por ID
                        // Para simplificar la validación de valencia al subir nivel:
                        if (getValenciaOcupada(idAtomo1) < getMaxValencia(idAtomo1) && // Nota: Usa una función auxiliar para buscar átomo por ID si los índices difieren
                            getValenciaOcupada(idAtomo2) < getMaxValencia(idAtomo2)) {

                            if (enlace.orden < 3) {
                                enlace.orden++;
                            } else {
                                listaEnlaces.removeAt(indiceEnlaceExistente); // Si pasa de triple, se elimina
                            }
                        } else {
                            if (enlace.orden == 3) listaEnlaces.removeAt(indiceEnlaceExistente); // Permitir eliminar incluso si está lleno
                        }
                    }
                    // 3. Si NO EXISTE, creamos un enlace simple nuevo (si las valencias lo permiten)
                    else {
                        // Al pasar directamente los IDs a las funciones auxiliares,
                        // ya no necesitas buscar manualmente el índice 'idx1' con el bucle for.
                        bool valencia1Ok = getValenciaOcupada(idAtomo1) < getMaxValencia(idAtomo1);
                        bool valencia2Ok = getValenciaOcupada(idAtomo2) < getMaxValencia(idAtomo2);

                        if (valencia1Ok && valencia2Ok) {
                            Enlace nuevoEnlace;
                            nuevoEnlace.id_atomo1 = idAtomo1;
                            nuevoEnlace.id_atomo2 = idAtomo2;
                            nuevoEnlace.orden = 1;
                            listaEnlaces.append(nuevoEnlace);
                        }
                    }
                    actualizarRenderizado(); // Refrescamos los gráficos en pantalla
                    emit contenidoModificado();
                }
                atomoSeleccionadoId = -1; // Liberamos selección
            }
        }
    }
    QGraphicsScene::mousePressEvent(mouseEv);
}

void LienzoMolecula::contextMenuEvent(QGraphicsSceneContextMenuEvent *menuEv){
    QPointF posClick = menuEv->scenePos();
    int idAtomoClick = buscarAtomoEnPosicion(posClick);

    if(idAtomoClick != -1){
        int idAtomoBorrar = listaAtomos[idAtomoClick].id;

        QMenu menu;
        QAction *actionEliminar = menu.addAction("Eliminar Átomo");
        QAction *actionSeleccionada = menu.exec(menuEv->screenPos());

        if(actionSeleccionada == actionEliminar){
            // 1. Eliminar todos los enlaces conectados a este átomo
            for (int i = listaEnlaces.size() - 1; i >= 0; --i) {
                if (listaEnlaces[i].id_atomo1 == idAtomoBorrar ||
                    listaEnlaces[i].id_atomo2 == idAtomoBorrar) {
                    listaEnlaces.removeAt(i);
                }
            }

            // 2. Eliminar el átomo de la lista lógica
            listaAtomos.removeAt(idAtomoClick);

            // 3. Forzar a Qt a redibujar la escena limpia
            actualizarRenderizado();

            // 4. Avisar a MainWindow de que el archivo ha cambiado (para poner el asterisco '*')
            emit contenidoModificado();
        }

    }
    else{
        // Si hace clic derecho en el vacío, pasamos el evento al comportamiento base
        QGraphicsScene::contextMenuEvent(menuEv);
    }
}


//###########################################################################################
// Funciones Privadas
//#########################################################################################

int LienzoMolecula::buscarAtomoEnPosicion(const QPointF &pos) const {
    double tolerancia = 20;

    for(int i=0; i<listaAtomos.size(); i++){

        double dx = listaAtomos[i].posicion.x() - pos.x();
        double dy = listaAtomos[i].posicion.y() - pos.y();

        double distancia = qSqrt((dx *dx) + (dy*dy));

        double radioMax = getRadioElemento(listaAtomos[i].simbolo) + tolerancia;

        if(distancia <= radioMax){
            return i;
        }
    }
    return -1;
}

QColor LienzoMolecula::getColorElemento(const QString &elemento) const{
    if (elemento == "H")  return Qt::white;
    if (elemento == "C")  return Qt::darkGray;
    if (elemento == "O")  return Qt::red;
    if (elemento == "N")  return Qt::blue;
    if (elemento == "S")  return Qt::yellow;
    if (elemento == "P")  return QColor(255, 165, 0); // Naranja
    if (elemento == "F" || elemento == "Cl") return Qt::green;
    return Qt::magenta; // Color por defecto para elementos no registrados
}

int LienzoMolecula::getRadioElemento(const QString &elemento) const{
    // Proporciones basadas en radios atómicos relativos (escalados para pantalla)
    if (elemento == "C") return 18; // Carbono estándar
    if (elemento == "H") return 12; // Hidrógeno (más pequeño)
    if (elemento == "O") return 16; // Oxígeno
    if (elemento == "N") return 17; // Nitrógeno
    return 15;
}

// Devuelve la valencia maxima de un elemento
int LienzoMolecula::getMaxValencia(int idAtomo) const {
    for (const Atomo &atomo : listaAtomos) {
        if (atomo.id == idAtomo) {
            QString simbolo = atomo.simbolo;
            if (simbolo == "H" || simbolo == "F" || simbolo == "Cl") return 1;
            if (simbolo == "O" || simbolo == "S") return 2;
            if (simbolo == "N" || simbolo == "P") return 3;
            if (simbolo == "C") return 4;
            return 0;
        }
    }
    return 0; // Si no encuentra el átomo por seguridad devuelve 0
}

// Suma todos los enlaces de un atomo ya ocupados
int LienzoMolecula::getValenciaOcupada(int idAtomo) const{
    int total = 0;
    for(const Enlace &enlace: listaEnlaces){
        if(enlace.id_atomo1 == idAtomo || enlace.id_atomo2 == idAtomo){
            total += enlace.orden;
        }
    }
    return total;
}

void LienzoMolecula::actualizarRenderizado(){
    // En lugar de añadir items de forma acumulativa, limpiamos la vista de Qt
    // y redibujamos el estado actual de los vectores lógicos.
    clear();

    // 1. Dibujamos los enlaces primero (para que queden por debajo de las esferas)
    for (const Enlace &enlace : listaEnlaces) {
        renderizarEnlace(enlace);
    }

    // 2. Dibujamos los átomos
    for (const Atomo &atomo : listaAtomos) {
        renderizarAtomo(atomo);
    }
}

void LienzoMolecula::renderizarAtomo(const Atomo &atomo){
    int     radio = getRadioElemento(atomo.simbolo);
    QColor  color = getColorElemento(atomo.simbolo);

    QGraphicsEllipseItem *circulo = addEllipse(
        atomo.posicion.x() - radio,
        atomo.posicion.y() - radio,
        radio * 2,
        radio * 2,
        QPen(Qt::black),
        QBrush(color)
        );
    circulo->setZValue(1);

    QGraphicsSimpleTextItem *texto = addSimpleText(atomo.simbolo);
    QRectF contornoTexto = texto->boundingRect();
    texto->setPos(
        atomo.posicion.x() - (contornoTexto.width() / 2),
        atomo.posicion.y() - (contornoTexto.height() / 2)
        );
    texto->setZValue(2); // Texto por encima de la esfera
}

void LienzoMolecula::renderizarEnlace(const Enlace &enlace){
    QPointF p1, p2;
    for(const Atomo &a : listaAtomos) {
        if(a.id == enlace.id_atomo1) p1 = QPointF(a.posicion.x(), a.posicion.y());
        if(a.id == enlace.id_atomo2) p2 = QPointF(a.posicion.x(), a.posicion.y());
    }

    if (enlace.orden == 1) {
        // Enlace Simple
        QGraphicsLineItem *linea = addLine(QLineF(p1, p2), QPen(Qt::black, 3));
        linea->setZValue(0);
    }
    else {
        // Cálculo del vector director y el vector perpendicular unitario para el desplazamiento
        QPointF dir = p2 - p1;
        double longitud = qSqrt(dir.x()*dir.x() + dir.y()*dir.y());
        if (longitud == 0) return;

        // Vector normalizado perpendicular (dx, dy) -> (-dy, dx)
        QPointF normal(-dir.y() / longitud, dir.x() / longitud);
        double distSeparacion = 5.0; // Píxeles de separación entre líneas paralelas

        if (enlace.orden == 2) {
            // Enlace Doble: Dos líneas desplazadas a cada lado del centro
            QPointF p1_a = p1 + normal * (distSeparacion / 2.0);
            QPointF p2_a = p2 + normal * (distSeparacion / 2.0);
            QPointF p1_b = p1 - normal * (distSeparacion / 2.0);
            QPointF p2_b = p2 - normal * (distSeparacion / 2.0);

            addLine(QLineF(p1_a, p2_a), QPen(Qt::black, 2))->setZValue(0);
            addLine(QLineF(p1_b, p2_b), QPen(Qt::black, 2))->setZValue(0);
        }
        else if (enlace.orden == 3) {
            // Enlace Triple: Una central y dos externas
            QPointF p1_a = p1 + normal * distSeparacion;
            QPointF p2_a = p2 + normal * distSeparacion;
            QPointF p1_b = p1 - normal * distSeparacion;
            QPointF p2_b = p2 - normal * distSeparacion;

            addLine(QLineF(p1, p2),     QPen(Qt::black, 2))->setZValue(0); // Central
            addLine(QLineF(p1_a, p2_a), QPen(Qt::black, 2))->setZValue(0); // Izquierda
            addLine(QLineF(p1_b, p2_b), QPen(Qt::black, 2))->setZValue(0); // Derecha
        }
    }
}
