#ifndef WIDGET_H
#define WIDGET_H

#include <vector>
#include <QWidget>
#include <QGraphicsView>
#define PI           3.14159265358979323846  /* pi */
#define TH           3.14159265358979323846  /* Todo, this is to be taken from user. This is the threshold per unit distance */


namespace Ui {
class Widget;
}

class Widget : public QWidget{
  Q_OBJECT
  typedef std::vector<double>   row_type;
  typedef std::vector<row_type> matrix_type;
  public:
    /* Sumit: What is the purpose of this line? */
    explicit Widget(QWidget* parent = 0);
    ~Widget();

  private:
    Ui::Widget* ui;
    QGraphicsScene* scene;
  private:
    matrix_type _matrix_in;
    matrix_type _matrix_out;
    matrix_type _matrix_scratch;
    unsigned int unfiltered_points;
    unsigned int _matrix_out_front;
    int last;
    void draw_marker(const QPointF& point, Qt::GlobalColor color = Qt::black);
    void draw_points(matrix_type matrix, Qt::GlobalColor color);
  private slots:/* TODO Sumit: What are the slots?? */
    void draw_paths();
    void take_input();
    void compute();
    void filter();
  private:
    double distance(const QPointF& p, const QPointF& q) const;/* Sumit what is the meaning of this const? */
    int location(const QPointF& p, const QPointF& q, const QPointF& r);
    double angle_per_distance(const QPointF &p, const QPointF &q, const QPointF &r);
    void draw_line(const QPointF& p, const QPointF& q, Qt::GlobalColor color);

};

#endif // WIDGET_H
