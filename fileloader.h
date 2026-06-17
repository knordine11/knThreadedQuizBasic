#ifndef FILELOADER_H
#define FILELOADER_H

#include <QIODevice>
#include <QMediaPlayer>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QUrl>
#include <QThread>

extern QString imgpath;
extern QString soundpath;
extern QString currentlesson;
extern QList<QString> gNote;
extern QList<QString> gKey;
extern QList<QString> gTestGroup;
extern QList<QString> noteFiles;
extern QList<int> testNotes;
extern QList<QByteArray> rawRecArrays;

class FileLoader : public QObject
{
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = nullptr);

    static void ReadConfig();
    static void ReadLesson();
    void GetFileList(int tonicNote);
    static void GetRandomTestSet(QString randomNotes);
    QString filenameChosen;
    QString filename;
    QAudioOutput *m_audioOutput = nullptr;
    QByteArray qb_rec_arr;
    int qb_size;

signals:

};

#endif // FILELOADER_H
