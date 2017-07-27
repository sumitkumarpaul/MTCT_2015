#include "geographicsview.h"
#include <cmath>
#include <QScrollBar>

GeoGraphicsView::GeoGraphicsView(QWidget* parent): QGraphicsView(parent){

}

GeoGraphicsView::GeoGraphicsView(QGraphicsScene* scene, QWidget* parent): QGraphicsView(scene, parent){

}

void GeoGraphicsView::wheelEvent(QWheelEvent* event){
    const QPointF p0scene = mapToScene(event->pos());
    qreal factor = std::pow(1.001, event->delta());
    scale(factor, factor);
    const QPointF p1mouse = mapFromScene(p0scene);
    const QPointF move = p1mouse - event->pos();
    horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
    verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());
}
