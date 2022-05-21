#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include"videoinformation.h"
#include<QPoint>
#include<QImage>
class GlobalData
{
public:
    GlobalData() ;
    ~GlobalData() ;
    static VideoInformation vi ;
    static QPoint qp ;
    static QImage qi ;
};

#endif // GLOBALDATA_H
