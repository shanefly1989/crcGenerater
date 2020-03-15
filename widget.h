#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QToolButton>
#include<QString>
#include<QTextStream>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;

public:
    void setToolButton(int index);
    void setPoly(int index);
    void updateButton();

    QVector<QToolButton*> toolbutton;

    void updatePoly();
    void updateCommonBox();
    int comparePoly();
    void generateCB();
    void generateRecult(QTextStream &out);
    void printHead(QTextStream &out);
    void printResult(int crcInt,int dataInt,QVector<QVector<int>*> &crcVector, QVector<QVector<int>*> &dataVector,QTextStream &out);
    void printModule(int crcInt, int dataInt,QTextStream &out);
    int getCrcInt();
    int getDataInt();

    int poly[68];

    int poly_pre_def[6][68] = { {1,1,1,0,1,1,0,1,1,0,1,1,1,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,1} ,
                                {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1} ,
                                {1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1} ,
                                {1,1,0,0,1,1,0,0,0,1,1} ,
                                {1,1,1,0,0,0,0,0,1},
                                {1,0,1,0,0,1} };
};

#endif // WIDGET_H
