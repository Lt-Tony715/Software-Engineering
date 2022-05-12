#include "GetMediaURLUi.h"
#include "ui_GetMediaURLUi.h"

GetMediaURLUi::GetMediaURLUi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetMediaURLUi)
{
    ui->setupUi(this);
}

GetMediaURLUi::~GetMediaURLUi()
{
    delete ui;
}

QString GetMediaURLUi::getDisplay() const
{
    return  ui->displayEditor->text();
}

QString GetMediaURLUi::getURL() const
{
    return  ui->urlEditor->text();
}
