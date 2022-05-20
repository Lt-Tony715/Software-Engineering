#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMediaPlayer>
#include <QMainWindow>
#include <QMap>
#include"videoinformation.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMediaPlayer;
class QMediaPlaylist;
class GetMediaURLUi;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpenLocalFile_triggered();

    void on_playList_doubleClicked(const QModelIndex &index);

    void on_playOrPauseButton_clicked();

    void on_stopButton_clicked();

    void on_playPosSlider_sliderPressed();

    void on_playPosSlider_sliderReleased();

    void on_playPosSlider_sliderMoved(int position);

    void on_playList_customContextMenuRequested(const QPoint &pos);

    void on_forwardButton_clicked() ;

    void on_listButton_clicked();


    void on_VolumeSlider_sliderMoved(int position);


private:
    void updatePlayText(bool play = true);
    void updatePlayText1(bool play = true);
    void updateDiplayPosInfo();

    void loadPlayList();
    void savePlayList();
    void keyPressEvent(QKeyEvent * event);
    void HideAndExpandPlayList() ;
    void VolumeUp();
    void VolumeDown();
    void Fast_forward_one_frame() ;
    void Fast_forward_five_frame() ;
    void mousePressEvent(QMouseEvent * event) ;

private:
    QMediaPlayer* m_mediaPlayer = nullptr;
    QVideoWidget* videoWidget = nullptr ;
    QMediaPlaylist* m_mediaPlayList = nullptr;

    QStringList m_nameList;
    QMap<QString, QString> m_ulrMap;

    QMenu* contenxMenu = nullptr;
    QMenu* c1 = nullptr ;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
