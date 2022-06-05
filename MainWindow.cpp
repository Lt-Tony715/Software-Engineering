#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QFileDialog>
#include <QTime>
#include <QThread>
#include <QTimer>
#include <QMessageBox>

#include <QDebug>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QKeyEvent>
#include<QMimeData>
#include<videoinformation.h>
#include"globaldata.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /***设置SpeedButtonBox***/
    ui->SpeedButtonBox->setCurrentIndex(1);
    //默认为1倍速播放

    ui->SpeedButton->setVisible(false);
    this->setAcceptDrops(true); //启动拖动事件
    ui->playList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->playPosSlider->setMaximum(5) ;
    ui->playPosSlider->setTracking(true);
    ui->VolumeSlider->setVisible(false);
    contenxMenu = new QMenu(this);
    contenxMenu->addAction("删除...", this, [this](){
        int pos = ui->playList->currentRow();
        QString name = m_nameList.value(pos);
        m_nameList.removeAt(pos);
        m_ulrMap.remove(name);
        m_mediaPlayList->removeMedia(pos);
        ui->playList->takeItem(pos);
        savePlayList();
    });
    contenxMenu->addAction("清空列表...", this, [this](){
        m_nameList.clear();
        m_ulrMap.clear();
        m_mediaPlayList->clear();
        ui->playList->clear();
        savePlayList();
    });
    contenxMenu->addAction("添加文件...", this, [this](){
        QString path = QFileDialog::getOpenFileName();
        QFileInfo fileInfo(path);
        QString url = fileInfo.absoluteFilePath();
        QString fileName = fileInfo.fileName();
        ui->playList->addItem(fileName);
        m_mediaPlayList->addMedia(QMediaContent(url));

        QTimer::singleShot(800, this, [this](){
            m_mediaPlayer->play();
        });

        m_nameList.append(fileName);
        m_ulrMap[fileName] = url;
        savePlayList();
    });

    videoWidget = new QVideoWidget(this);
    //videoWidget = new myvideowidget(this) ;
    ui->playWidgetLayout->addWidget(videoWidget);

    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setVideoOutput(videoWidget);

    m_mediaPlayList = new QMediaPlaylist(this);
    m_mediaPlayer->setPlaylist(m_mediaPlayList);
    m_mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
    m_mediaPlayer->setPlaybackRate(qreal(1));
//    connect(videoWidget, &QVideoWidget::customContextMenuRequested, this, [this](const QPoint &pos){
//        c1->exec(ui->playList->mapToGlobal(pos));
//    }) ;
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State newState) {
        if (newState == QMediaPlayer::PlayingState)
        {
            updatePlayText(false);
        }
        else
        {
            updatePlayText(true);
        }
    });

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        ui->playPosSlider->setValue(position);
        updateDiplayPosInfo();
    });

    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        if(m_mediaPlayList->currentIndex() == -1){
            return ;
        }
        int index = m_mediaPlayList->currentIndex() ;
        QString name = ui->playList->item(index)->text();
        QString s_temp = m_ulrMap[name] ;
        GlobalData::vi.getVideoInfo(s_temp) ;
        //GlobalData::vi.test1() ;
        //qDebug()<<GlobalData::vi.getFrameRate()<<endl ;
        //GlobalData::vi.getVideoInfo(m_mediaPlayer)
        ui->playPosSlider->setMaximum(duration);
        ui->playPosSlider->setValue(0);
        ui->pic->setVisible(false);
        if(GlobalData::vi.pic_flag())
        {
            QImage pic("1.jpg");
            ui->pic->setVisible(true);
            ui->pic->setPixmap(QPixmap::fromImage(pic));;
        }
        else
        {
            ui->pic->setVisible(false);
        }
    });

    connect(m_mediaPlayer, (void (QMediaPlayer::*)(QMediaPlayer::Error))&QMediaPlayer::error, this, [this](QMediaPlayer::Error error) {
        Q_UNUSED(error)
        ui->statusbar->showMessage(m_mediaPlayer->errorString());
    });

    //自定义的信号costomSliderClicked()
    connect(ui->playPosSlider,SIGNAL(costomSliderClicked()),this,SLOT(sliderClicked()));

    connect(ui->VolumeSlider, SIGNAL(costomSliderClicked()), this, SLOT(volume_slider_clicker())) ;

    connect(ui->playPosSlider, SIGNAL(painte()), this, SLOT(paint_image())) ;

    updatePlayText(true);
    loadPlayList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenLocalFile_triggered()
{
   QString path = QFileDialog::getOpenFileName();
   qDebug()<<path<<endl ;
   QFileInfo fileInfo(path);
   QString url = fileInfo.absoluteFilePath();
   QString fileName = fileInfo.fileName();

   ui->playList->addItem(fileName);
   m_mediaPlayList->addMedia(QMediaContent(url));

   QTimer::singleShot(800, this, [this](){
       m_mediaPlayer->play();
   });

   m_nameList.append(fileName);
   m_ulrMap[fileName] = url;
   savePlayList();
}

void MainWindow::on_playList_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return ;
    }
    QString name = ui->playList->item(index.row())->text();
    //qDebug()<<name<<endl ;

    if (m_ulrMap.contains(name)){
            QFile file(m_ulrMap[name]);
            if(file.exists()){
                ui->statusbar->showMessage("播放成功");
            }
            else{
                ui->statusbar->showMessage("播放失败，没有对应的ulr地址");
                if (QMessageBox::warning(this, "警告", "当前影视不能播放，是否删除", "是", "否") == 0)
                {
                    m_nameList.removeAll(name);
                    m_ulrMap.remove(name);
                    ui->playList->takeItem(index.row());
                    savePlayList();
                }
                m_mediaPlayer->pause();
                return ;
            }
      }
//    if (!m_ulrMap.contains(name))
//    {
//        ui->statusbar->showMessage("播放失败，没有对应的ulr地址");
//        if (QMessageBox::warning(this, "警告", "当前影视不能播放，是否删除", "是", "否") == 0)
//        {
//            m_nameList.removeAll(name);
//            m_ulrMap.remove(name);
//            ui->playList->takeItem(index.row());
//            savePlayList();
//        }
//        return ;
//    }
    m_mediaPlayList->setCurrentIndex(index.row());

    QTimer::singleShot(800, this, [this](){
        m_mediaPlayer->play();
    });
}

void MainWindow::on_listButton_clicked()
{
    HideAndExpandPlayList() ;
}

void MainWindow::on_playOrPauseButton_clicked()
{
    if (m_mediaPlayer->state() != QMediaPlayer::PlayingState)
    {
//        m_mediaPlayer->setPlaybackRate(qreal(4));
        m_mediaPlayer->play();

    }
    else
    {
        m_mediaPlayer->pause();

    }
}

void MainWindow::on_stopButton_clicked()
{
    m_mediaPlayer->stop();
}

//上一首
void MainWindow::on_PreButton_clicked()
{
    if(m_mediaPlayList->currentIndex()==0)
    {
        m_mediaPlayList->setCurrentIndex(m_mediaPlayList->mediaCount()-1);
        ui->playList->setCurrentRow(m_mediaPlayList->mediaCount()-1);
        m_mediaPlayer->play();
        return ;
    }
    m_mediaPlayList->previous();
    ui->playList->setCurrentRow(m_mediaPlayList->currentIndex());
    m_mediaPlayer->play();
}

//下一首
void MainWindow::on_pushButton_clicked()
{
    if(m_mediaPlayList->currentIndex()==m_mediaPlayList->mediaCount()-1)
    {
        m_mediaPlayList->setCurrentIndex(0);
        ui->playList->setCurrentRow(0);
        m_mediaPlayer->play();
        return ;
    }
    m_mediaPlayList->next();
    ui->playList->setCurrentRow(m_mediaPlayList->currentIndex());
    m_mediaPlayer->play();
}

void MainWindow::on_playPosSlider_sliderPressed()
{
    m_mediaPlayer->pause();
}

void MainWindow::on_playPosSlider_sliderReleased()
{
    m_mediaPlayer->play();
}

void MainWindow::on_playPosSlider_sliderMoved(int position)
{
    m_mediaPlayer->setPosition(position);
}

void MainWindow::on_playList_customContextMenuRequested(const QPoint &pos)
{
    contenxMenu->exec(ui->playList->mapToGlobal(pos));
}


void MainWindow::on_PlayModeButton_clicked()
{
    if(m_mediaPlayList->playbackMode()==QMediaPlaylist::Loop)
    {
        m_mediaPlayList->setPlaybackMode(QMediaPlaylist::Random);
        ui->PlayModeButton->setText("列表随机");
    }
    else if(m_mediaPlayList->playbackMode()==QMediaPlaylist::Random)
    {
        m_mediaPlayList->setPlaybackMode(QMediaPlaylist::Sequential);
        ui->PlayModeButton->setText("列表顺序");
    }
    else if(m_mediaPlayList->playbackMode()==QMediaPlaylist::Sequential)
    {
        m_mediaPlayList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        ui->PlayModeButton->setText("单曲循环");
    }
    else if(m_mediaPlayList->playbackMode()==QMediaPlaylist::CurrentItemInLoop)
    {
        m_mediaPlayList->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
        ui->PlayModeButton->setText("单曲播放");
    }
    else if(m_mediaPlayList->playbackMode()==QMediaPlaylist::CurrentItemOnce)
    {
        m_mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
        ui->PlayModeButton->setText("列表循环");
    }
}

//void MainWindow::on_SpeedButton_clicked()
//{
//    qDebug()<<m_mediaPlayer->playbackRate()<<endl;
//    if(m_mediaPlayer->playbackRate()==qreal(1))
//    {
//        m_mediaPlayer->setPlaybackRate(qreal(2));
//        ui->SpeedButton->setText("x2");
//    }
//    else if(m_mediaPlayer->playbackRate()==qreal(2))
//    {
//        m_mediaPlayer->setPlaybackRate(qreal(4));
//        ui->SpeedButton->setText("x4");
//    }
//    else if(m_mediaPlayer->playbackRate()==qreal(4))
//    {
//        m_mediaPlayer->setPlaybackRate(qreal(0.5));
//        ui->SpeedButton->setText("x0.5");
//    }
//    else if(m_mediaPlayer->playbackRate()==qreal(0.5))
//    {
//        m_mediaPlayer->setPlaybackRate(qreal(1));
//        ui->SpeedButton->setText("x1");
//    }
//}

void MainWindow::updatePlayText(bool play)
{
    ui->playOrPauseButton->setText(play ? "播放" : "暂停");
}

void MainWindow::on_VolumeSlider_sliderReleased()
{
    ui->VolumeSlider->setVisible(false);
}


void MainWindow::on_VolumeButton_clicked()
{
    if(ui->VolumeSlider->isVisible())
    {
        ui->VolumeSlider->setVisible(false);
    }else
    {
        ui->VolumeSlider->setVisible(true);
    }

}

void MainWindow::updateDiplayPosInfo()
{
    auto convertDispalyStr = [](int totalMilliseconds) -> QString {
        QTime tm = QTime(0, 0, 0, 0).addMSecs(totalMilliseconds);
        return  tm.toString("hh:mm:ss");
    };
    const QString position = convertDispalyStr(ui->playPosSlider->value());
    const QString duration = convertDispalyStr(ui->playPosSlider->maximum());
    ui->playPosLable->setText(QString("%1 / %2").arg(position).arg(duration));
}

void MainWindow::loadPlayList()
{
    QFile file("config/config.json");
    if (!file.exists())
    {
        ui->statusbar->showMessage("不存在配置文件...");
        return ;
    }
    if (!file.open(QFile::ReadOnly))
    {
        ui->statusbar->showMessage("打开配置文件用于加载失败...");
        return ;
    }
    QJsonArray jsonArray = QJsonDocument::fromJson(file.readAll()).array();
    for (QJsonValue value : jsonArray)
    {
        QJsonObject jsonObject = value.toObject();
        QString name = jsonObject.value("name").toString();
        QString url = jsonObject.value("url").toString();
//        qDebug()<<url<<endl ;
//        QFile f(url);
//        if(!f.exists()){
//            continue ;
//        }
        m_nameList.append(name);
        m_ulrMap[name] = url;
        m_mediaPlayList->addMedia(QMediaContent(url));
    }
    ui->playList->addItems(m_nameList);    
    file.close();
}

void MainWindow::savePlayList()
{
    QJsonArray jsonArray;
    for (QString name : m_nameList)
    {
        QJsonObject jsonObject;
        jsonObject["name"] = name;
        jsonObject["url"] = m_ulrMap.value(name);
        jsonArray.append(jsonObject);
    }
    QDir dir("config");
    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());

    }
    QFile file(dir.filePath("config.json"));

    if (!file.open(QFile::WriteOnly))
    {
        ui->statusbar->showMessage("打开配置文件用于存储失败...");
        return ;
    }
    file.write(QJsonDocument(jsonArray).toJson());
    file.close();
}

void MainWindow::on_forwardButton_clicked()
{
    this->Fast_forward_five_frame() ;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier)
    {
        //如果是，那么再检测M键是否按下
        if(event->key() == Qt::Key_F)
        {
            //按下则使窗口最大化
            this->setWindowState(Qt::WindowMaximized);
        }
        if(event->key() == Qt::Key_M)
        {
            HideAndExpandPlayList() ;
        }
        if(event->key() == Qt::Key_Up)
        {
            VolumeUp()  ;
        }
        if(event->key() == Qt::Key_Down)
        {
            VolumeDown()  ;
        }
        if(event->key() == Qt::Key_P)
        {
            this->Fast_forward_five_frame() ;
        }
        if(event->key() == Qt::Key_Q)
        {
            this->Fast_forward_one_frame() ;
        }
        if(event->key() == Qt::Key_L)
        {
            this->HideAndExpandLayout() ;
        }
        if(event->key() == Qt::Key_Left)
        {
            if(m_mediaPlayList->currentIndex()==0)
            {
                m_mediaPlayList->setCurrentIndex(m_mediaPlayList->mediaCount()-1);
                m_mediaPlayer->play();
                return ;
            }
            m_mediaPlayList->previous();
            m_mediaPlayer->play();
        }
        if(event->key() == Qt::Key_Right)
        {
            if(m_mediaPlayList->currentIndex()==m_mediaPlayList->mediaCount()-1)
            {
                m_mediaPlayList->setCurrentIndex(0);
                m_mediaPlayer->play();
                return ;
            }
            m_mediaPlayList->next();
            m_mediaPlayer->play();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::RightButton)
    {
        c1 = new QMenu(this) ;
        if(GlobalData::vi.r_flag()){
            c1->addAction(new QAction("视频信息", this)) ;
            QString Info_name="文件路径：";
            Info_name+=GlobalData::vi.getname() ;
            QString Info_bit_rate="码率：";
            Info_bit_rate+=QString::number(GlobalData::vi.getVideoAverageBitRate()) ;
            //QString Info_codec="编解码器：";
            //Info_codec+=VideoVector[FindAudioOrVideoByName(filename)].codec;

            QString Info_frame_rate="帧率：";
            Info_frame_rate+=QString::number(GlobalData::vi.getFrameRate()) ;
            QString Info_resolution="分辨率：";
            Info_resolution+=QString::number(GlobalData::vi.getWidth()) ;
            Info_resolution+="*" ;
            Info_resolution+=QString::number(GlobalData::vi.getHeight()) ;

            c1->addAction(new QAction(Info_name,this));
            c1->addAction(new QAction(Info_resolution,this));
            c1->addAction(new QAction(Info_frame_rate,this));
            c1->addAction(new QAction(Info_bit_rate,this));
            //ContextMenu->addAction(new QAction(Info_codec,this));
        }
        else {
            c1->addAction(new QAction("音频信息", this)) ;
            QString Info_name="文件路径：";
            Info_name+=GlobalData::vi.getname() ;
            QString info_bit_rate = "码率： " ;
            info_bit_rate+=QString::number(GlobalData::vi.getAudioAverageBitRate()) ;
            QString info_format = "编码格式: " ;
            info_format+=GlobalData::vi.getAudioFormat() ;
            QString info_s_number = "声道数： " ;
            info_s_number+=QString::number(GlobalData::vi.getChannelNumbers()) ;


            c1->addAction(new QAction(Info_name,this));
            c1->addAction(new QAction(info_bit_rate,this));
            c1->addAction(new QAction(info_format,this));
            c1->addAction(new QAction(info_s_number,this));
        }


        c1->exec(event->globalPos());
    }
}

void MainWindow::HideAndExpandPlayList()
{
    if(ui->playList->isVisible()==true)
    {
        ui->playList->setVisible(false);
        ui->listButton->setText("展开");

    }
    else
    {
        ui->playList->setVisible(true);
        ui->listButton->setText("隐藏");
    }
}

void MainWindow::HideAndExpandLayout()
{
    if(ui->playPosSlider->isVisible()==true)
    {
        ui->playPosSlider->setVisible(false);
        ui->VolumeSlider->setVisible(false);
        ui->forwardButton->setVisible(false);
        ui->VolumeButton->setVisible(false);
        ui->listButton->setVisible(false);
        ui->playOrPauseButton->setVisible(false);
        ui->stopButton->setVisible(false);
        ui->playPosLable->setVisible(false);
        ui->PlayModeButton->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->PreButton->setVisible(false);
        ui->SpeedButtonBox->setVisible(false);
    }
    else
    {
        ui->playPosSlider->setVisible(true);
        ui->VolumeSlider->setVisible(true);
        ui->forwardButton->setVisible(true);
        ui->VolumeButton->setVisible(true);
        ui->listButton->setVisible(true);
        ui->playOrPauseButton->setVisible(true);
        ui->stopButton->setVisible(true);
        ui->playPosLable->setVisible(true);
        ui->PlayModeButton->setVisible(true);
        ui->pushButton->setVisible(true);     
        ui->PreButton->setVisible(true);
        ui->SpeedButtonBox->setVisible(true);
    }
}

void MainWindow::VolumeUp()
{
    int value = ui->VolumeSlider->value() ;
    value=value+5>99?99:value+5  ;
    ui->VolumeSlider->setValue(value)  ;
    m_mediaPlayer->setVolume(value)  ;
}

void MainWindow::VolumeDown()
{
    int value = ui->VolumeSlider->value() ;
    value=value-5<0?0:value-5  ;
    ui->VolumeSlider->setValue(value)  ;
    m_mediaPlayer->setVolume(value)  ;
}

void MainWindow::on_VolumeSlider_sliderMoved(int position)
{
    m_mediaPlayer->setVolume(position);
}

void MainWindow::Fast_forward_one_frame()
{
    m_mediaPlayer->pause() ;
    qint64 ps = m_mediaPlayer->position()+1000/GlobalData::vi.getFrameRate() ;
    while(m_mediaPlayer->position() < ps){
        m_mediaPlayer->play() ;
    }
    m_mediaPlayer->pause();
}

void MainWindow::Fast_forward_five_frame()
{
    m_mediaPlayer->pause() ;
    qint64 ps = m_mediaPlayer->position()+5000/GlobalData::vi.getFrameRate() ;
    while(m_mediaPlayer->position() < ps){
        m_mediaPlayer->play() ;
    }
    m_mediaPlayer->pause();
}

/*
* slider点击事件发生后，改变视频的进度
*/
void MainWindow::sliderClicked(){
    m_mediaPlayer->setPosition(ui->playPosSlider->value());
}

void MainWindow::volume_slider_clicker(){
    m_mediaPlayer->setVolume(ui->VolumeSlider->value())  ;
}

void MainWindow::paint_image()
{
    //repaint() ;
    qDebug()<<"i am here"<<endl ;
    videoWidget->update() ;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString str = event->mimeData()->text() ;
    //qDebug()<<event->mimeData()->text()<<endl;
    str.remove(0, 8) ;
    qDebug()<<str<<endl ;
    QFileInfo fileInfo(str);
    QString url = fileInfo.absoluteFilePath();
    //qDebug()<<url<<endl ;
    QString fileName = fileInfo.fileName();

    ui->playList->addItem(fileName);
    m_mediaPlayList->addMedia(QMediaContent(url));

    QTimer::singleShot(800, this, [this](){
        m_mediaPlayer->play();
    });

    m_nameList.append(fileName);
    m_ulrMap[fileName] = url;
    savePlayList();
}
//void MainWindow::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);
//    QPixmap Qpix ;
//    Qpix.load("D:\\finall\\bin\\1.jpg") ;
//    Qpix = Qpix.scaled(300, 300, Qt::KeepAspectRatio);
//    painter.drawPixmap(GlobalData::qp.rx(), GlobalData::qp.ry()+300, Qpix);

//}

void MainWindow::on_SpeedButtonBox_activated(int index)
{
    qDebug()<<m_mediaPlayer->playbackRate()<<endl;
    if(index == 0){
        m_mediaPlayer->setPlaybackRate(qreal(0.5));
    }
    else if(index == 1){
        m_mediaPlayer->setPlaybackRate(qreal(1));
    }
    else if(index == 2){
        m_mediaPlayer->setPlaybackRate(qreal(2));
    }
    else{
        m_mediaPlayer->setPlaybackRate(qreal(4));
    }
}

