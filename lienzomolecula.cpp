#include "lienzomolecula.h"

#include <QPointF>
#include <QString>
#include <QObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QLine>
#include <QtMath>


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

QString LienzoMolecula::generarContenidoMOPAC(const QString &argumentos) const{
    // Primera y segunda línea: Argumentos y título obligatorio para MOPAC
    QString contenido = argumentos + "\n";
    contenido += "Molecula generada con VisorMol_V10\n\n";

    // Escribimos la tabla de coordenadas cartesianas (Matriz Z simplificada)
    for (const Atomo &atomo : listaAtomos) {
        // Formato estándar MOPAC: Simbolo X 1 Y 1 Z 1
        // (Los '1' le indican a MOPAC que optimice geométricamente esa coordenada)
        contenido += QString("%1   %2 1   %3 1   0.0000 1\n")
                         .arg(atomo.simbolo)
                         .arg(atomo.posicion.x() / 50.0, 0, 'f', 4)  // Escalamos los píxeles a Angstroms aproximados
                         .arg(atomo.posicion.y() / 50.0, 0, 'f', 4);
    }
    return contenido;
}

void LienzoMolecula::mousePressEvent(QGraphicsSceneMouseEvent *mouseEv){

    // 1. Extraemos la posición exacta (X, Y) del clic en el lienzo
    QPointF posClick = mouseEv->scenePos();

    // 2.- Comprobamos si existe atomo en el click
    int hayAtomoPosClick = buscarAtomoEnPosicion(posClick);

    // Modo Añadir un Atomo
    if(modoEnlaceActivo == false){
        // el Click ha sido en espacio vacío ???
        if(hayAtomoPosClick == -1){
            Atomo nuevoAtomo;

            nuevoAtomo.id       = contadorIds++;
            nuevoAtomo.simbolo  = elementoActual;
            nuevoAtomo.posicion = posClick;

            listaAtomos.append(nuevoAtomo);

            int     radio = getRadioElemento(nuevoAtomo.simbolo);
            QColor  color = getColorElemento(nuevoAtomo.simbolo);

            QGraphicsEllipseItem *circulo = addEllipse(
                posClick.x() - radio,
                posClick.y() - radio,
                radio * 2,
                radio * 2,
                QPen(Qt::black),
                QBrush(QColor(getColorElemento(nuevoAtomo.simbolo)))
                );
            circulo->setZValue(1);

            QGraphicsSimpleTextItem *texto = addSimpleText(nuevoAtomo.simbolo);
            QRectF contornoTexto = texto->boundingRect();
            texto->setPos(
                posClick.x() - (contornoTexto.width() / 2),
                posClick.y() - (contornoTexto.height() / 2)
                );
            texto->setZValue(2); // Texto por encima de la esfera
        }
    }
    // Modo Crear un enlace
    else{
        if(hayAtomoPosClick != -1){ // El clic debe ser sobre un átomo válido
            int idAtomoClick = listaAtomos[hayAtomoPosClick].id;

            if(atomoSeleccionadoId == -1){ // No hay atomo selecionado anterior
                atomoSeleccionadoId = idAtomoClick;
            }
            else{ // Segundo atomo para el enlace
                if(idAtomoClick != atomoSeleccionadoId){
                    bool existeEnlace = false;

                    for(const Enlace &enlace: listaEnlaces){
                        if((enlace.id_atomo1 == idAtomoClick && enlace.id_atomo2 == atomoSeleccionadoId) ||
                            (enlace.id_atomo1 == atomoSeleccionadoId && enlace.id_atomo2 == idAtomoClick)){
                            existeEnlace = true;
                            break;
                        }
                    }

                    if(!existeEnlace){
                        // Creamos el enlace
                        Enlace nuevoEnlace;

                        nuevoEnlace.id_atomo1 = atomoSeleccionadoId;
                        nuevoEnlace.id_atomo2 = idAtomoClick;
                        listaEnlaces.append(nuevoEnlace);

                        // Dibujamos el enlace
                        QPointF pos1;
                        QPointF pos2 = listaAtomos[hayAtomoPosClick].posicion;

                        for(const Atomo &atomo: listaAtomos){
                            if(atomo.id == atomoSeleccionadoId){
                                pos1 = atomo.posicion;
                                break;
                            }
                        }

                        QGraphicsLineItem *linea = addLine(QLineF(pos1, pos2), QPen(Qt::black, 3));
                        linea->setZValue(0);
                    }
                }
                // Liberamos el primer atomo seleccionado
                atomoSeleccionadoId = -1;
            }
        }
    }


    // Protip de Qt: Invocamos al método base para que la escena pueda gestionar
    // correctamente otros eventos internos del sistema si hiciera falta.
    QGraphicsScene::mousePressEvent(mouseEv);
}

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
