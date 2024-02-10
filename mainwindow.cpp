#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    int iconSize = 25;

    QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    QIcon skipIcon = style()->standardIcon(QStyle::SP_MediaSeekForward);
    QIcon backIcon = style()->standardIcon(QStyle::SP_MediaSeekBackward);
    QIcon volumeIcon = style()->standardIcon((QStyle::SP_MediaVolume));

    // Get the actual size of the icon without scaling
    QSize actualSize = playIcon.actualSize(QSize(iconSize, iconSize));

    // set size for button
    ui->toggleButton_PlayPause->setIcon(playIcon);
    ui->toggleButton_PlayPause->setIconSize(actualSize);
    ui->pushButton_Skip->setIcon(skipIcon);
    ui->pushButton_Skip->setIconSize(actualSize);
    ui->pushButton_Back->setIcon(backIcon);
    ui->pushButton_Back->setIconSize(actualSize);
    ui->toggleButton_Mute->setIcon(volumeIcon);
    ui->toggleButton_Mute->setIconSize(actualSize);

    this->M_Player = new QMediaPlayer();
    this->audio_Output = new QAudioOutput();

    M_Player->setAudioOutput(audio_Output);
    M_Duration = M_Player->duration() / 1000;

    // Default volume
    ui->slider_SongVolume->setMinimum(0);
    ui->slider_SongVolume->setMaximum(100);
    ui->slider_SongVolume->setValue(50);
    audio_Output->setVolume(0.5);

    // Connect volume to slider
    connect(ui->slider_SongVolume, &QSlider::sliderMoved, this, &MainWindow::on_slider_SongVolume_sliderMoved);

    // Duration slider
    connect(M_Player, &QMediaPlayer::durationChanged, this, &MainWindow::durationChanged);
    connect(M_Player, &QMediaPlayer::positionChanged, this, &MainWindow::positionChanged);

    ui->slider_SongProgress->setRange(0, M_Player->duration() / 1000);

    // Import songs from computer
    connect(ui->pushButton_Songs, &QPushButton::clicked, this, &MainWindow::addAllSongs);

    // Play song when clicked in Playlist
    connect(ui->listWidget_Songs_in_Playlist, &QListWidget::itemClicked, this, &MainWindow::on_listWidget_Songs_in_Playlist_itemClicked);

    // Skip song button
    connect(ui->pushButton_Skip, &QPushButton::clicked, this, &MainWindow::skipToNextSong);

    // Back song button
    connect(ui->pushButton_Back, &QPushButton::clicked, this, &MainWindow::BackToPreviousSong);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete M_Player;
    delete audio_Output;
}

void MainWindow::durationChanged(qint64 duration)
{
    M_Duration = duration / 1000;
    ui->slider_SongProgress->setMaximum(M_Duration);
}

void MainWindow::positionChanged(qint64 progress)
{
    if (!ui->slider_SongProgress->isSliderDown()) {
        ui->slider_SongProgress->setValue(progress / 1000);
    }
    updateDuration(progress / 1000);
}

void MainWindow::updateDuration(qint64 duration)
{
    if (duration || M_Duration) {
        QTime currentTime((duration / 3600) % 60, (duration / 60) % 60, duration % 60, (duration * 1000) % 1000);
        QTime totalTime((M_Duration / 3600) % 60, (M_Duration / 60) % 60, M_Duration % 60, (M_Duration * 1000) % 1000);
        QString format = "mm:ss";
        if (M_Duration > 3600) {
            format = "hh:mm:ss";
        }
        ui->label_CurrentSongDuration->setText(currentTime.toString(format));
        ui->label_TotalSongDuration->setText(totalTime.toString(format));
    }
}

void MainWindow::on_toggleButton_Mute_clicked()
{
    if (!isMuted) {
        ui->toggleButton_Mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        audio_Output->setMuted(true);  // Mute the audio
        isMuted = true;
    }
    else {
        ui->toggleButton_Mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        audio_Output->setMuted(false);  // Unmute the audio
        isMuted = false;
    }
}


void MainWindow::on_toggleButton_PlayPause_clicked()
{
    if (!isPaused) {
        ui->toggleButton_PlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        M_Player->play();
        isPaused = true;
    }
    else {
        ui->toggleButton_PlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        M_Player->pause();
        isPaused = false;
    }
}

void MainWindow::on_actionAdd_File_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Open a file", "", "Audio File (*.mp3)");

    if (!file_name.isEmpty()) {
        M_Player->setSource(QUrl::fromLocalFile(file_name));

        if (M_Player->mediaStatus() != QMediaPlayer::NoMedia) {
            // Media loaded successfully
            QFileInfo fileInfo(file_name);
            QString fileNameWithoutExtension = fileInfo.fileName();
            fileNameWithoutExtension = fileNameWithoutExtension.left(fileNameWithoutExtension.lastIndexOf('.')); // Remove the file extension
            ui->label_fileName->setText(fileNameWithoutExtension);
        }
        else {
            // Handle error loading media
            qDebug() << "Error loading media file:" << M_Player->errorString();
        }
    }
}

void MainWindow::on_slider_SongProgress_sliderMoved(int value)
{
    M_Player->setPosition(static_cast<qint64>(value) * 1000);
}


void MainWindow::on_slider_SongVolume_sliderMoved(int value)
{
    audio_Output->setVolume(value / 100.0);

    if (value == 0) {
        ui->toggleButton_Mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
    else {
        ui->toggleButton_Mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }
}

void MainWindow::on_pushButton_AddPlaylist_clicked()
{
    // Open a dialog to get the name of the new item from the user
    QString newPlaylistName = QInputDialog::getText(this, tr("Add Playlist"), tr("Enter the name of the new playlist:"));

    // Handle Error
    if (!newPlaylistName.isEmpty()) {
        // Create a new item with the text entered by the user
        QListWidgetItem* newPlaylist = new QListWidgetItem(newPlaylistName);

        // Add item to playlist
        ui->listWidget_Playlist->addItem(newPlaylist);
    }
}
void MainWindow::on_pushButton_RemovePlaylist_clicked()
{
    // Get the currently selected item
    QListWidgetItem* item = ui->listWidget_Playlist->currentItem();

    // Remove the item from the list
    delete item;

}

void MainWindow::addAllSongs() {
    // Clear the list widget first
    ui->listWidget_Songs_in_Playlist->clear();

    // Directory containing music files (change this to your desired directory)
    QString directory = "/Users/pannatatsribusarakham/Documents/Music";

    // Get all music files from the directory
    QStringList musicFilters;
    musicFilters << "*.mp3" << "*.wav";

    QDir musicDir(directory);
    QFileInfoList musicFiles = musicDir.entryInfoList(musicFilters, QDir::Files);

    for (const QFileInfo &fileInfo : musicFiles) {
        // Get the file name without extension
        QString fileName = fileInfo.fileName();
        fileName = fileName.left(fileName.lastIndexOf('.')); // Remove the file extension

        QListWidgetItem *musicItem = new QListWidgetItem(fileName);
        ui->listWidget_Songs_in_Playlist->addItem(musicItem);
    }
    ui->label_PlaylistName->setText("All Songs");
    setFileCountToLabel(directory, ui->label_TrackQuantity);
}

void MainWindow::on_listWidget_Songs_in_Playlist_itemClicked(QListWidgetItem *item)
{
    QString filePath = "/Users/pannatatsribusarakham/Documents/Music/" + item->text() + ".mp3";
    M_Player->setSource(QUrl::fromLocalFile(filePath));

    if(M_Player->mediaStatus() != QMediaPlayer::NoMedia) {
        M_Player->play();
    } else {
        qDebug() << "Error setting media source: " << M_Player->errorString();
    }

    QString fileName = item->text();
    fileName = fileName.left(fileName.lastIndexOf('.'));
    ui->label_fileName->setText(fileName);
    ui->toggleButton_PlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
}

void MainWindow::setFileCountToLabel(const QString &folderPath, QLabel *label)
{
    QDir folderDir(folderPath);
    QFileInfoList fileInfoList = folderDir.entryInfoList(QDir::Files);
    int numFiles = fileInfoList.size();
    label->setText(QString::number(numFiles)); // Convert integer to QString
}

void MainWindow::skipToNextSong() {
    // Get the current row
    int currentRow = ui->listWidget_Songs_in_Playlist->currentRow();

    // Get the number of items in the list widget
    int itemCount = ui->listWidget_Songs_in_Playlist->count();

    // Check if there are items in the list widget
    if (itemCount > 0) {
        int nextRow = (currentRow + 1) % itemCount;

        // Select the next item in the list widget
        ui->listWidget_Songs_in_Playlist->setCurrentRow(nextRow);

        // Play the next song
        QListWidgetItem *nextItem = ui->listWidget_Songs_in_Playlist->currentItem();

        if (nextItem) {
            QString fileName = nextItem->text();
            ui->label_fileName->setText(fileName);
            QString songFilePath = "/Users/pannatatsribusarakham/Documents/Music/" + nextItem->text() + ".mp3";
            M_Player->setSource(QUrl::fromLocalFile(songFilePath));
            M_Player->play();
        }

    }
}

void MainWindow::BackToPreviousSong() {
    int currentRow = ui->listWidget_Songs_in_Playlist->currentRow();
    int itemCount = ui->listWidget_Songs_in_Playlist->count();
    if (itemCount > 0) {
        int previousRow = (currentRow - 1) % itemCount;

        ui->listWidget_Songs_in_Playlist->setCurrentRow(previousRow);

        QListWidgetItem *previousItem = ui->listWidget_Songs_in_Playlist->currentItem();

        if(previousItem) {
            QString fileName = previousItem->text();
            ui->label_fileName->setText(fileName);
            QString songFilePath = "/Users/pannatatsribusarakham/Documents/Music/" + previousItem->text() + ".mp3";
            M_Player->setSource(QUrl::fromLocalFile(songFilePath));
            M_Player->play();
        }
    }
}
