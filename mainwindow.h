#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>
#include <QFileDialog>
#include <QStyle>
#include <QAudioOutput>
#include <QListWidget>
#include <QInputDialog>
#include <QIcon>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void durationChanged(qint64 duration);

    void positionChanged(qint64 progress);

    void on_toggleButton_PlayPause_clicked();

    void on_toggleButton_Mute_clicked();

    void on_actionAdd_File_triggered();

    void on_slider_SongProgress_sliderMoved(int value);

    void on_slider_SongVolume_sliderMoved(int value);

    void on_pushButton_AddPlaylist_clicked();

    void on_pushButton_RemovePlaylist_clicked();

    void addAllSongs();

    void on_listWidget_Songs_in_Playlist_itemClicked(QListWidgetItem *item);

    void updateDuration(qint64 duration);

    void skipToNextSong();

    void BackToPreviousSong();

    void SkipWhenSongEnd(QMediaPlayer::MediaStatus status);

private:
    Ui::MainWindow* ui;

    bool isMuted = false;

    bool isPaused = false;

    QMediaPlayer* M_Player;

    QAudioOutput* audio_Output;

    qint64 M_Duration;

    void setFileCountToLabel(const QString &folderPath, QLabel *label);

};
#endif // MAINWINDOW_H
