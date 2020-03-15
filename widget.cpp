#include "widget.h"
#include "ui_widget.h"
#include<QFileDialog>
#include<QMessageBox>
#include<QTextStream>
#include<QDebug>
#include<QComboBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    setWindowTitle("crc generate");

    int cnt = 0;

    for(int i=0;i<4;i++)
    {
        for(int j=0;j<17;j++)
        {
            QToolButton *btn = new QToolButton;
            QString name = "x";
            name += QString::number(cnt);
            btn->setText(name);
            btn->setCheckable(true);
            btn->setMinimumSize(60,60);

            if(i==0&&j==0)
            {
                btn->setChecked(true);
                connect(btn,&QToolButton::clicked,[=](){
                    btn->setChecked(true);
                    updateCommonBox();

                });
            }
            else
                connect(btn,&QToolButton::clicked,this,&Widget::updateCommonBox);

            ui->button_layout->addWidget(btn,i,j);
            toolbutton.push_back(btn);

            cnt++;
        }
    }

    ui->data_width->setText("1");
    ui->but_verilog->setChecked(true);

    connect(ui->predef_val,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&Widget::setToolButton);

    connect(ui->generate,&QPushButton::clicked,this,&Widget::generateCB);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::updatePoly()
{
    QVector<QToolButton*>::iterator it;
    int cnt=0;
    for(it = toolbutton.begin();it!=toolbutton.end();it++)
    {
        if((*it)->isChecked())
        {
            poly[cnt] = 1;
        }
        else
        {
            poly[cnt] = 0;
        }
        cnt++;
    }
}

void Widget::setToolButton(int index)
{


    if(index == 0)
        return;
    setPoly(index-1);
    updateButton();
}

void Widget::setPoly(int index)
{
    for(int i=0;i<68;i++)
    {
        poly[i] = poly_pre_def[index][i];
    }
}

void Widget::updateButton()
{
    for(int i=0;i<68;i++)
    {
        if(poly[i] == 1)
        {
            toolbutton[i]->setChecked(true);
        }
        else
        {
            toolbutton[i]->setChecked(false);
        }
    }
}

void Widget::generateCB()
{
    updatePoly();
    int crcInt = getCrcInt();
    int dataInt = getDataInt();
    if(crcInt == 0 || dataInt<=0)
    {
        QMessageBox::warning(this,"warning","请输入正确的参数！！！！");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this,"存储文件",".","*.v");
    if(path == "")
        return;
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"warning","打开的文件错误！！！！");
    }
    else
    {
        QTextStream out(&file);
        generateRecult(out);
        file.close();
    }
}

void Widget::updateCommonBox()
{

    updatePoly();
    int index=comparePoly();
    ui->predef_val->setCurrentIndex(index);

}

int Widget::comparePoly()
{
    for(int i=0;i<6;i++)
    {
        for(int j=0;j<68;j++)
        {
            if(poly[j] != poly_pre_def[i][j])
            {
                break;
            }
            if(j==67)
                return i+1;
        }
    }
    return 0;
}

void Widget::generateRecult(QTextStream &out)
{
    int crcInt;
    int dataInt;
    printHead(out);

    crcInt = getCrcInt();
    dataInt = getDataInt();

    QVector<QVector<int>*> crcVector;
    QVector<QVector<int>*> dataVector;

    for(int i=0;i<crcInt;i++)
    {
        QVector<int> *crc = new QVector<int>;
        crc->resize(crcInt);
        (*crc)[i] = 1;
        crcVector.push_back(crc);

        QVector<int> *data = new QVector<int>;
        data->resize(dataInt);
        dataVector.push_back(data);
    }

    for(int i=dataInt-1;i>=0;i--)
    {
        //产生dataIn
        QVector<int> dataIn;
        dataIn.resize(dataInt);
        dataIn[i] = 1;

        for(int i=0;i<dataInt;i++)
        {
            dataIn[i] = dataIn[i] ^ (*dataVector[crcInt-1])[i];
        }

        //产生crcIn
        QVector<int> crcIn;
        crcIn.resize(crcInt);
        for(int k=0;k<crcInt;k++)
        {
            crcIn[k] = (*crcVector[crcInt-1])[k];
        }

        for(int j=crcInt-1;j>=1;j--)
        {

            if(poly[j] == 1)
            {
                for(int k=0;k<crcInt;k++)
                {
                    (*crcVector[j])[k] = ((*crcVector[j-1])[k] ^ crcIn[k]);
                }
                for(int k=0;k<dataInt;k++)
                {
                    (*dataVector[j])[k] = ((*dataVector[j-1])[k] ^ dataIn[k]);
                }
            }
            else
            {
                for(int k=0;k<crcInt;k++)
                {
                    (*crcVector[j])[k] = (*crcVector[j-1])[k];
                }
                for(int k=0;k<dataInt;k++)
                {
                    (*dataVector[j])[k] = (*dataVector[j-1])[k];
                }
            }
        }

        for(int k=0;k<crcInt;k++)
        {
            (*crcVector[0])[k] = crcIn[k];

        }
        for(int k=0;k<dataInt;k++)
        {
            (*dataVector[0])[k] = dataIn[k];
        }
    }


    printModule(crcInt,dataInt,out);
    printResult(crcInt,dataInt,crcVector,dataVector,out);
    //out << QString::number(crcInt);
    //out << QString::number(dataInt);
}

void Widget::printHead(QTextStream &out)
{
    out << "///////////////////////////////////////"<<"\n";
    out << "//Copyright by shenlu."<<"\n";
    out << "///////////////////////////////////////"<<"\n";
}

int Widget::getCrcInt()
{
    for(int i=67;i>=0;i--)
    {
        if(poly[i] == 1)
            return i;
    }
}

int Widget::getDataInt()
{
    return ui->data_width->text().toInt();
}

void Widget::printResult(int crcInt,int dataInt,QVector<QVector<int>*> &crcVector, QVector<QVector<int>*> &dataVector,QTextStream &out)
{
    out << "    wire [" << QString::number(dataInt-1) << ":0] d;\n";
    out << "    wire [" << QString::number(crcInt-1) << ":0] c;\n";
    out << "    reg [" << QString::number(crcInt-1) << ":0] crc_out;\n";
    out << "\n\n";
    out << "    assign d = data;\n";
    out << "    assign c = crc_in;\n";
    out << "\n\n";
    out << "    always@(*)\n";
    out << "    begin\n";
    for(int i=0;i<crcInt;i++)
    {
        bool firstOne = true;
        out << "        crc_out[" << QString::number(i) << "] <= ";
        for(int j=dataInt-1;j>=0;j--)
        {
            if((*dataVector[i])[j] == 1)
            {
                if(firstOne)
                {
                    out << "d[" << QString::number(j) << "]";
                    firstOne = false;
                }
                else
                {
                    out << " ^ d[" << QString::number(j) << "]";
                }
            }
        }
        for(int j=0;j<crcInt;j++)
        {
            if((*crcVector[i])[j] == 1)
            {
                if(firstOne)
                {
                    out << "c[" << QString::number(j) << "]";
                    firstOne = false;
                }
                else
                {
                    out << " ^ c[" << QString::number(j) << "]";
                }
            }
        }
        out << ";\n";
    }

    out << "    end\n";
    out << "endmodule\n";

}

void Widget::printModule(int crcInt, int dataInt,QTextStream &out)
{
    QString moduleName;
    moduleName = tr("module ") + tr("CRC") +
                 QString::number(crcInt) + tr("_D") +
                 QString::number(dataInt) + tr(" (\n");
    moduleName = moduleName + tr("    input clk,\n");
    moduleName = moduleName + tr("    input [") + QString::number(dataInt-1) + tr(":0] data,\n");
    moduleName = moduleName + tr("    input [") + QString::number(crcInt-1) + tr(":0] crc_in,\n");
    moduleName = moduleName + tr("    output [") + QString::number(crcInt-1) + tr(":0] crc_out\n");
    moduleName = moduleName + tr(");\n\n");
    QString zhushi;
    zhushi = tr("// polynomial: ") + tr("x^") + QString::number(crcInt);
    for(int i=crcInt-1;i>=0;i--)
    {
        if(poly[i] == 1)
        {
            zhushi = zhushi + tr(" + x^") + QString::number(i);
        }
    }
    zhushi += "\n";
    zhushi = zhushi + tr("// data width: ") + QString::number(dataInt) + tr("\n");
    zhushi = zhushi + tr("// convention: the first serial bit is D[") + QString::number(dataInt-1) + tr("]\n");
    out << zhushi;
    out << "\n";
    out << moduleName;
}
