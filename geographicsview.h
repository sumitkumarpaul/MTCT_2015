#ifndef GEOGRAPHICSVIEW_H
#define GEOGRAPHICSVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QWheelEvent>

class GeoGraphicsView : public QGraphicsView{
  public:
    GeoGraphicsView(QWidget* parent= 0);
    GeoGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);
  public:
    virtual void wheelEvent(QWheelEvent* event);
};

#endif // GEOGRAPHICSVIEW_H
