#include "fileloader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRandomGenerator>

QString imgpath;
QString soundpath;
QString currentlesson;
QList<QString> gNote;
QList<QString> gKey;
QList<QString> gTestGroup;
QList<QString> noteFiles;
QList<int> testNotes;
QList<QByteArray> rawRecArrays;

FileLoader::FileLoader(QObject *parent)
    : QObject{parent}
{

}

void FileLoader::ReadConfig()
{
    QFile file(":/data/config.txt");

    if(!file.exists())
    {
        qInfo() << "file not found";
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        qCritical("Open failed");
    }
    QTextStream stream(&file);
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        qInfo() << line;
        QStringList items = line.split(',');
        if(items[1].contains("graphic"))
        {
            imgpath = items[0];
        }
        if(items[1].contains("sound"))
        {
            soundpath = items[0];
        }
        if(items[1].contains("Current"))
        {
            currentlesson = items[0];
        }
    }
    file.close();
}

void FileLoader::ReadLesson()
{
    QFile file(":/data/lessons.txt");

    if(!file.exists())
    {
        qInfo() << "file not found";
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        qCritical("Failed to open");
    }
    QTextStream stream(&file);
    stream.readLine();
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        QStringList items = line.split('|');
        gNote.append(items[0] + items[1]);
        gKey.append(items[0]);

        gTestGroup.append(items[4]);
    }
    qInfo() << gNote;
    qInfo() << gTestGroup;
    file.close();
}

void FileLoader::GetFileList(int startingNote)
{

    //QList<QString> noteFiles;
    QString noteFile = "v" + QString::number(startingNote) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 2) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 4) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 5) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 7) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 9) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 11) + ".wav";
    noteFiles.append(noteFile);
    noteFile = "v" + QString::number(startingNote + 12) + ".wav";
    noteFiles.append(noteFile);

    qDebug() << noteFiles;
    QString filenameChosen = ":/data/v_sounds/" + noteFile;

    for(int i=0;i<8;i++)
    {
        QFile in(":/data/v_sounds/" + noteFiles[i]);
        if (!in.open(QIODevice::ReadOnly))
        {
            qDebug() << "open failed";
        }
        qb_rec_arr = in.readAll();
        qb_size = qb_rec_arr.size();
        // strip the header convert to float
        QByteArray temp = qb_rec_arr.slice(44, qb_size - 44);
        qDebug() << temp.size();
        rawRecArrays.append(temp);
    }
}

void FileLoader::GetRandomTestSet(QString randomNotes)
{
    qDebug() << "test group = " << randomNotes;
    QList<int> seeds;
    QStringList items = randomNotes.split(',');
    for(QString item : items)
    {
        qDebug() << item;
        int temp = item.toInt();
        seeds.append(temp);
    }
    qDebug() << seeds;
    // QList<int> testNotes[20];
    for(int i=0; i<20; i++)
    {
        int index = rand() % seeds.size();
        int value = seeds[index];
        qDebug() << value;
        testNotes.append(value);
    }
}
