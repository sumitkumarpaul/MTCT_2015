#include "widget.h"
#include "ui_widget.h"
#include <QRectF>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>
#include <QStringList>
#include <QDebug>
#include <QString>
#include <iostream>
#include <limits>
#include <vector>
#include <cmath>
#include <QFileDialog>
#include <QTextStream>
#include <QTransform>

Widget::Widget(QWidget* parent): QWidget(parent), ui(new Ui::Widget), last(0){
    ui->setupUi(this);

    scene = new QGraphicsScene;
    ui->view->setScene(scene);
    ui->computeBtn->setEnabled(false);
    connect(ui->computeBtn, SIGNAL(released()), this, SLOT(filter()));
    connect(ui->inputBtn, SIGNAL(released()), this, SLOT(take_input()));
    connect(ui->filter, SIGNAL(released()), this, SLOT(filter()));
    ui->computeBtn->setEnabled(false);
    ui->filter->setEnabled(false);

    colormap.insert(std::make_pair(marker_observation, Qt::green));
    colormap.insert(std::make_pair(marker_important, Qt::red));
    colormap.insert(std::make_pair(marker_redundant, Qt::gray));
    colormap.insert(std::make_pair(marker_delete, Qt::blue));
}

Widget::~Widget(){
    delete ui;
    delete scene;
}

void Widget::draw_paths(){
    draw_points(_matrix_in, marker_observation);
}

void Widget::draw_points(matrix_type matrix, marker_state state){
    unsigned int index = 1;
    foreach(row_type row, matrix){
        QPointF point(QPointF(row[0], row[1]));
        QGraphicsEllipseItem* circle = find_marker(point);
        if(!circle){
            qDebug() << "NOT FOUND " << point;
            circle = draw_marker(point, index++, colormap[state]);
        }else{
            qDebug() << "FOUND " << point;
            change_state(circle, state);
        }
    }
}

QGraphicsEllipseItem* Widget::draw_marker(const QPointF& point, unsigned int index, Qt::GlobalColor color){
    QPen pen(color);
    QBrush brush(color);
    QRectF rect(point, QSizeF(5, 5));
    QGraphicsEllipseItem* circle = scene->addEllipse(rect, pen, brush);
    circle->setToolTip(QString("(%1,%2)").arg(point.x()).arg(point.y()));
    if(index){
        QFont sansFont("Helvetica [Cronyx]", 7);
        QGraphicsSimpleTextItem* text_item = scene->addSimpleText(QString("%1").arg(index), sansFont);
        QPointF text_point(point);
        text_point.setX(point.x());
        text_point.setY(point.y());
        text_item->setPos(text_point);
    }
    markersmap.insert(std::make_pair(point, circle));
    return circle;
}

void Widget::draw_marker(const QPointF& point, Qt::GlobalColor color){
    draw_marker(point, 0, color);
}

QGraphicsEllipseItem* Widget::find_marker(const QPointF &point) const{
     QGraphicsEllipseItem* circle = 0x0;
     markers_map_type::const_iterator it = markersmap.find(point);
     if(it != markersmap.end()){
         circle = it->second;
     }
     return circle;
}

void Widget::change_state(QGraphicsEllipseItem* marker, marker_state state){
    qDebug() << marker << state;
    if(marker){
        QPen pen(colormap[state]);
        QBrush brush(colormap[state]);
        marker->setBrush(brush);
        marker->setPen(pen);
        if(state == marker_delete){
            scene->removeItem(marker);
            markers_map_type::iterator it = markersmap.find(marker->rect().topLeft());
            markersmap.erase(it);

            QList<QGraphicsItem*> items = scene->items(marker->rect().topLeft());
            foreach(QGraphicsItem* item, items){
                QGraphicsSimpleTextItem* textitem = dynamic_cast<QGraphicsSimpleTextItem*>(item);
                if(textitem){
                    scene->removeItem(textitem);
                    delete textitem;
                    textitem = 0x0;
                }
            }
            delete marker;
            marker = 0x0;
        }
    }
}

void Widget::draw_line(const QPointF& p, const QPointF& q, Qt::GlobalColor color){
    scene->addLine(p.x(), p.y(), q.x(), q.y(), QPen(color, 1, Qt::SolidLine, Qt::RoundCap));
    return;
}

void Widget::take_input(){
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Spreadsheets (*.csv)"));
    QFile f(file_name);
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&f);
    QString text = in.readAll();
    QStringList lines =text.split("\n", QString::SkipEmptyParts);
    std::vector<std::vector<double>> matrix;
    foreach(QString line, lines){
        QStringList values_str = line.split(",", QString::SkipEmptyParts);
        std::vector<double> row;
        foreach(QString value, values_str){
            row.push_back(value.toDouble());
        }
        matrix.push_back(row);
    }
    _matrix_in = matrix;
    draw_paths();
    ui->inputBtn->setEnabled(false);
    ui->computeBtn->setEnabled(true);

    std::cout << "Input button is pressed..!!" << std::endl;
    qDebug("Input button is pressed..!!");
    merge();
}

void Widget::merge(){
    matrix_type matrix_intermediate;
    int existing_row_cnt = 0;
    int next_idx_to_be_traversed_in_out_matrix = 0;
    std::cout << "Compute button is pressed..!!" << std::endl;

    /* Findout the number of rows in the existing matrix */
    existing_row_cnt = _matrix_out.size();

    /* Set each new input points(_matrix_in) into the existing point list(_matrix_out) */
    for(matrix_type::const_iterator i = _matrix_in.begin(); i != _matrix_in.end(); ++i){
        row_type rowi = *i;
        QPointF p(rowi[0], rowi[1]);
        double last_d = std::numeric_limits<double>::max();
        double d1,d2;
        int remaining_untraversed_elements;

        /* remaining_untraversed_elements Index of last element - index of the next element to be traveresed + 1*/
        remaining_untraversed_elements = ((existing_row_cnt - 1) - next_idx_to_be_traversed_in_out_matrix) + 1;
        /* If there is two or more untraversed elements in _matrix_out */
        if (remaining_untraversed_elements >= 2)
        {
            last_d = std::numeric_limits<double>::max();

            /* If new point is in the left of row = next_idx_to_be_traversed_in_out_matrix */
            QPointF q1(_matrix_out[next_idx_to_be_traversed_in_out_matrix][0],_matrix_out[next_idx_to_be_traversed_in_out_matrix][1]);
            QPointF q2(_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][0],_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][1]);
            d1 = distance(p, q1);
            d2 = distance(p, q2);

            while((last_d > (d1 + d2)) && (remaining_untraversed_elements >= 2))
            {
                matrix_intermediate.push_back(_matrix_out[next_idx_to_be_traversed_in_out_matrix]);
                last_d = d1 + d2;
                next_idx_to_be_traversed_in_out_matrix++;
                remaining_untraversed_elements--;

                if (remaining_untraversed_elements >= 2)
                {
                    /* If new point is in the left of row = next_idx_to_be_traversed_in_out_matrix */
                    QPointF q1(_matrix_out[next_idx_to_be_traversed_in_out_matrix][0],_matrix_out[next_idx_to_be_traversed_in_out_matrix][1]);
                    QPointF q2(_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][0],_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][1]);
                    d1 = distance(p, q1);
                    d2 = distance(p, q2);
                }
            }

            if(remaining_untraversed_elements == 1)
            {
                goto only_one_untraversed_element_remaining;
            }
            else/* Already optimum postion is passed */
            {
                int pos;

                /* Restore back the optimum position */
                next_idx_to_be_traversed_in_out_matrix--;
                (void)matrix_intermediate.pop_back();
                /*******************************************************************
                 * So now, q = _matrix_out[next_idx_to_be_traversed_in_out_matrix]
                 * r = _matrix_out[next_idx_to_be_traversed_in_out_matrix + 1]
                 ******************************************************************/
                /****************************************************************************************
                 * _matrix_out: |1st element|2nd element|...|prev element of q| q | r |...|last element|
                 *                                                              ^
                 *                                             next_idx_to_be_traversed_in_out_matrix
                 *
                 *
                ****************************************************************************************/
                QPointF p(rowi[0], rowi[1]);
                QPointF q(_matrix_out[next_idx_to_be_traversed_in_out_matrix][0], _matrix_out[next_idx_to_be_traversed_in_out_matrix][1]);
                QPointF r(_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][0], _matrix_out[next_idx_to_be_traversed_in_out_matrix + 1][1]);

                /* Findout the position of p, with respect to q and r */
                pos = location(p,q,r);

                /* If p is at the left of q */
                if (pos == 0)
                {
                    /* Push only the new point(p) */
                    matrix_intermediate.push_back(rowi);
                }
                else if(pos == 1)/* Else if p is in-between q and r */
                {
                    /* Push q then p */
                    matrix_intermediate.push_back(_matrix_out[next_idx_to_be_traversed_in_out_matrix]);
                    matrix_intermediate.push_back(rowi);
                }
                else/* Else p is at the right of r */
                {
                    /* Push q then r then p */
                    matrix_intermediate.push_back(_matrix_out[next_idx_to_be_traversed_in_out_matrix]);
                    matrix_intermediate.push_back(_matrix_out[next_idx_to_be_traversed_in_out_matrix + 1]);
                    matrix_intermediate.push_back(rowi);

                    next_idx_to_be_traversed_in_out_matrix++;
                }

            }
        }
        else if(remaining_untraversed_elements == 1)/* There is only one element in _matrix_out */
        {
only_one_untraversed_element_remaining:
            int last_idx = (matrix_intermediate.size() - 1);

            if (last_idx == -1)/* i.e no element existin the intermediate matrix */
            {
                /* Nothing to calculate, push this point */
                matrix_intermediate.push_back(rowi);
            }
            else
            {
                int pos;
                QPointF p(rowi[0], rowi[1]);
                QPointF q(matrix_intermediate[last_idx][0],matrix_intermediate[last_idx][1]);
                QPointF r(_matrix_out[next_idx_to_be_traversed_in_out_matrix][0],_matrix_out[next_idx_to_be_traversed_in_out_matrix][1]);

                /* Findout the position of p, with respect to q and r */
                pos = location(p,q,r);

                /* If p is at the left of q */
                if (pos == 0)
                {
                    /* Push only the new point(p) */
                    std::cout << "This should not happen..!!" << std::endl;
                }
                else if(pos == 1)/* Else if p is in-between q and r */
                {
                    matrix_intermediate.push_back(rowi);
                }
                else/* Else p is at the right of r */
                {
                    /* Push q then r then p */
                    matrix_intermediate.push_back(_matrix_out[next_idx_to_be_traversed_in_out_matrix]);
                    matrix_intermediate.push_back(rowi);

                    next_idx_to_be_traversed_in_out_matrix++;
                }

            }
        }
        else/* There is no element in _matrix_out */
        {
            /* Push p */
            matrix_intermediate.push_back(rowi);
        }
    }


    _matrix_out = matrix_intermediate;
    _matrix_out_front = 0;

    /* For debugging purpose only */
    for (unsigned int row = 0; row < _matrix_out.size(); ++row)
    {
        for (unsigned int col = 0; col < _matrix_out[row].size(); ++col)
        {
            std::cout << _matrix_out[row][col] << " ";
        }
        std::cout << std::endl;
    }

    unfiltered_points = _matrix_out.size();
    /* Enable the filter button at the end of the function */
    ui->filter->setEnabled(true);
    /* Disable nothing to compute */


    return;
}

double Widget::distance(const QPointF &p, const QPointF &q) const{
    return std::sqrt(
                std::pow(p.x() - q.x(), 2)+
                std::pow(p.y() - q.y(), 2)
                );
}
/***************************************************************
 *           r
 *          /
 *         /
 * p-----q
 *
 * ************************************************************/
double Widget::angle_per_distance(const QPointF &p, const QPointF &q, const QPointF &r)
{
    double m1 = (((p.y())-(q.y()))/((p.x())-(q.x())));
    double m2 = (((q.y())-(r.y()))/((q.x())-(r.x())));

    return (((180 * std::abs(std::atan2((m2-m1) , (1 + (m1 * m2))))) / PI) / distance(q,r));
}


/************************************************************************
 *
 *
 * **********************************************************************/
int Widget::location(const QPointF& p, const QPointF& q,const QPointF& r)
{
    int ret = 0;

    double m = ((q.y() - r.y())/(q.x() - r.x()));/* Gradient of existing line */
    double m_t = (-1/m);/* Gradient of the tangent to the existing line */

    double x = ((p.y() - q.y()) + ((q.x()*m) - (p.x()*m_t)))/(m - m_t);
    double y = q.y() + m*(x - q.x());

    QPointF s(x,y);

    if ((distance(s,q) + distance(s,r)) == distance(r,q))/*--q----s---r---*/
    {
        ret = 1;/* S is in the middle */
    }
    else if(distance(s,r) == distance(r,q) + distance(s,q))/*--s----q---r---*/
    {
        ret = 0;/* S at the left */
    }
    else if(distance(s,q) == distance(r,q) + distance(s,r))/*----q---r-s--*/
    {
        ret = 2;/* S at the right */
    }
    else
    {
        std::cout << "The code should not come here..!!" << std::endl;
        ret = -1;
    }

    return ret;
}



void Widget::filter()
{
    matrix_type matrix_intermediate;

//    if (_matrix_scratch.size() == 0)
//    {
//        scene->clear();
//    }

    if (unfiltered_points > 0)
    {
        /* For first two data, no need to computation, just add the data */
        if(_matrix_scratch.size() > 2)
        {
            QPointF p(_matrix_scratch[_matrix_scratch.size() - 2][0],_matrix_scratch[_matrix_scratch.size() - 2][1]);
            QPointF q(_matrix_scratch[_matrix_scratch.size() - 1][0],_matrix_scratch[_matrix_scratch.size() - 1][1]);
            QPointF r(_matrix_out[_matrix_out_front][0],_matrix_out[_matrix_out_front][1]);

            if(angle_per_distance(p,q,r) <= TH)
            {
                draw_line(p,q, Qt::gray);
                draw_line(q,r, Qt::gray);
                draw_line(p,r, Qt::red);
                _matrix_scratch.pop_back();
                QPointF point(QPointF(_matrix_scratch[_matrix_scratch.size() - 1][0], _matrix_scratch[_matrix_scratch.size() - 1][1]));
                QGraphicsEllipseItem* circle = find_marker(point);
                change_state(circle, marker_redundant);
//                draw_marker(QPointF(_matrix_scratch[_matrix_scratch.size() - 1][0], _matrix_scratch[_matrix_scratch.size() - 1][1]), Qt::gray);
            }
            else
            {
                draw_line(p,q, Qt::red);
                draw_line(q,r, Qt::red);
            }

        }
        _matrix_scratch.push_back(_matrix_out[_matrix_out_front]);
//        draw_marker(QPointF(_matrix_out[_matrix_out_front][0], _matrix_out[_matrix_out_front][1]), Qt::red);
        QGraphicsEllipseItem* circle = find_marker(QPointF(_matrix_out[_matrix_out_front][0], _matrix_out[_matrix_out_front][1]));
        change_state(circle, marker_important);

        _matrix_out_front++;
        unfiltered_points--;
    }

    if (_matrix_scratch.size() >= 2)
    {
        QPointF p(_matrix_scratch[_matrix_scratch.size() - 2][0],_matrix_scratch[_matrix_scratch.size() - 2][1]);
        QPointF q(_matrix_scratch[_matrix_scratch.size() - 1][0],_matrix_scratch[_matrix_scratch.size() - 1][1]);

        draw_line(p,q, Qt::red);
    }


    if(unfiltered_points == 0)
    {
        draw_points(_matrix_out, marker_delete);

        /* Set filtered value in out matrix */
        _matrix_out = _matrix_scratch;

        draw_points(_matrix_out, marker_important);

        _matrix_out_front = 0;
        /* Erase the scratch matrix */
        _matrix_scratch.erase(_matrix_scratch.begin(), _matrix_scratch.end());

        /* Disable, no points to filter */
        ui->filter->setEnabled(false);
        ui->computeBtn->setEnabled(false);
        /* User can now give input again */
        ui->inputBtn->setEnabled(true);
    }

    return;
}

bool operator<(const QPointF& lhs, const QPointF& rhs) {
    return (lhs.x() < rhs.x()) || ((lhs.x() == rhs.x()) && (lhs.y() < rhs.y()));
}
