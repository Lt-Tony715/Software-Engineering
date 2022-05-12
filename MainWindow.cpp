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
#include<videoinformation.h>
#include"globaldata.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->playList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->playPosSlider->setMaximum(5) ;
    ui->playPosSlider->setTracking(true);
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

    QVideoWidget* videoWidget = new QVideoWidget(this);
    ui->playWidgetLayout->addWidget(videoWidget);

    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setVideoOutput(videoWidget);

    m_mediaPlayList = new QMediaPlaylist(this);
    m_mediaPlayer->setPlaylist(m_mediaPlayList);

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
        ui->playPosSlider->setMaximum(duration);
        ui->playPosSlider->setValue(0);
    });

    connect(m_mediaPlayer, (void (QMediaPlayer::*)(QMediaPlayer::Error))&QMediaPlayer::error, this, [this](QMediaPlayer::Error error) {
        Q_UNUSED(error)
        ui->statusbar->showMessage(m_mediaPlayer->errorString());
    });

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
   QString url = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toString();
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
    if (!m_ulrMap.contains(name))
    {
        ui->statusbar->showMessage("播放失败，没有对应的ulr地址");
        if (QMessageBox::warning(this, "警告", "当前影视不能播放，是否删除", "是", "否") == 0)
        {
            m_nameList.removeAll(name);
            m_ulrMap.remove(name);
            ui->playList->takeItem(index.row());
            savePlayList();
        }
        return ;
    }
//    QString s_temp = m_ulrMap[name] ;
//    s_temp.remove(0, 8) ;
//    //qDebug()<<s_temp<<endl ;
//    GlobalData::vi.getVideoInfo(s_temp) ;
//    qDebug()<<GlobalData::vi.getFrameRate()<<endl ;
    m_mediaPlayList->setCurrentIndex(index.row());

    QTimer::singleShot(800, this, [this](){
        m_mediaPlayer->play();
    });
}
void MainWindow::on_listButton_clicked()
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

void MainWindow::updatePlayText(bool play)
{
    ui->playOrPauseButton->setText(play ? "播放" : "暂停");
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
    int value = ui->playPosSlider->value() ;
    QList<QListWidgetItem*> temp = ui->playList->selectedItems() ;
    if(temp.size() <= 0){
        return ;
    }
//    QString name = ui->playList->selectedItems()[0]->text() ;
//    QString s_temp = m_ulrMap[name] ;
//    s_temp.remove(0, 8) ;
//    qDebug()<<s_temp<<endl ;
//    GlobalData::vi.getVideoInfo(s_temp) ;
//    qDebug()<<GlobalData::vi.getFrameRate()<<endl ;
    int m_unit = m_mediaPlayer->notifyInterval() ;
    m_mediaPlayer->setPosition(value+m_unit);
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

    }
}
