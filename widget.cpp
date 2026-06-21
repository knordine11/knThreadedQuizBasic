#include "widget.h"
#include "ui_widget.h"
#include "fileloader.h"
#include <QAudioDevice>
#include <QAudioSource>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QtEndian>
#include "fftstuff.h"
#include <qtimer.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <QMessageBox>

extern bool collectMicData;
extern double rec_arr[];    // DEFINED AS DOUBLE FOR FFTW
extern int rec_arr_cnt;
extern int frame_start;
extern int frame_size;
extern int frame_end;
extern int rec_arr_end;
const QList <QString> note_letters = {"C", "C#", "D", "D#", "E",
                                     "F", "F#", "G", "G#", "A", "A#", "B" };
int curLessonInt = 0;
int orientation [21] = {1,2,3,4,5,6,7,8,1,8,1,8,1,8,7,6,5,4,3,2,1};
QMap<QString, int> tonic_map = {
    {"G3", 43}, {"A3", 45}, {"B3", 47}, {"C4", 48}, {"D4", 50}, {"A4", 52}, {"B4", 54}, {"C5", 55}
};
QList<int> kbNotePlayLists;
Speaker speaker;
QThread speakerThread;
extern QList<QByteArray> rawRecArrays;
FftStuff ftw;
int accValue = 0;
int displayDuration = 3000;


Microphone::Microphone(const QAudioFormat &format) : m_format(format) {
    qDebug()<<" YOU SHOULD SEE THIS ";
}

Microphone::~Microphone()
{

}

void Microphone::start()
{
    open(QIODevice::WriteOnly);
}

void Microphone::stop()
{
    close();
}

qint64 Microphone::readData(char * /* data */, qint64 /* maxlen */)      // NOT USED IN EXAMPLE
{
    return 0;
}

qreal Microphone::getNoteValue(const char *data, qint64 len) const
{
    const int channelBytes = m_format.bytesPerSample();
    const int sampleBytes = m_format.bytesPerFrame();
    const int numSamples = len / sampleBytes;

    float maxValue = 0;
    auto *ptr = reinterpret_cast<const unsigned char *>(data);

    for (int i = 0; i < numSamples; ++i) {
        float value = m_format.normalizedSampleValue(ptr);
        rec_arr[rec_arr_cnt]=value;
        //maxValue = qMax(value, maxValue);
        ptr += channelBytes;
        rec_arr_cnt++;
    };
    if(rec_arr_cnt > frame_end){
        cout<<"\n  NEXT FRAME $$$$ "<<frame_start<<endl;

        ftw.DoIt(frame_start, frame_size);
        frame_start = frame_end;
        frame_end = frame_end + frame_size;}
    if (rec_arr_cnt > 200000)
    {
        qDebug() <<"                      restart here";
        rec_arr_cnt = 0;

        emit on_TimeOut();
        qDebug() <<"                      post restart here";
        return 0;
    }
    return maxValue;
}

qint64 Microphone::writeData(const char *data, qint64 len)
{
    // qDebug() << "enter writeData" << rec_arr_cnt;
    m_level = getNoteValue(data, len);
    return len;
}

Speaker::Speaker()
{

}

void Speaker::start()
{
    open(QIODevice::ReadOnly);
}

void Speaker::stop()
{
    m_pos = 0;
    close();
}

void Speaker::newTest(QByteArray bufferOut)
{
    qDebug()<<"  %%%%  NEW TEST ";
    m_buffer.assign(bufferOut);
    m_buffer.clear();
    qDebug()<<&m_buffer<<" m_buffer_.size() " << m_buffer.size();
    m_buffer = bufferOut;
    qDebug()<<&m_buffer<<" m_buffer_.size() " << m_buffer.size();
}

qint64 Speaker::readData(char *data, qint64 len)
{
    qDebug() << "enter speaker readdata...is on main? " << QThread::isMainThread();
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
            qDebug() << "chunk..." << chunk << "pos> = " << m_pos ;
        }
    }
    return total;
}

qint64 Speaker::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Speaker::bytesAvailable() const
{
    return QIODevice::bytesAvailable();
}

void Speaker::startSound()
{
    qDebug() << "is Main Thread in startSound: " << QThread::isMainThread();

}

void Speaker::stopSound()
{
    stop();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("Basic Quiz test");
    initializeWindow();
    initializeAudio(QMediaDevices::defaultAudioInput());
    initializeAudioOutput(m_devicesOut->defaultAudioOutput());
    ui->btnStop->setVisible(false);
    ui->btnNext->setVisible(false);
    FileLoader::ReadConfig();
    FileLoader::ReadLesson();
    FileLoader::GetRandomTestSet(gTestGroup[curLessonInt]);
    // QString temp = "Lesson #" + currentlesson + " Key " + gKey[curLessonInt-1]
    //                + " Test Notes " + gTestGroup[curLessonInt];
    ui->lbCurrentLesson->setText(" Test Notes " + gTestGroup[curLessonInt]);

    FftStuff fts;
    orientationFlag = true;
    connect(&ftw, &FftStuff::valueChanged,this, &Widget::updateKBnote, Qt::QueuedConnection);
    connect(&ftw, &FftStuff::on_foundNote,this, &Widget::Got_Note, Qt::QueuedConnection);
    connect(m_Microphone, &Microphone::on_TimeOut, this, &Widget::TimeOut);
}

Widget::~Widget()
{
    MicThread.terminate();
    delete m_audioSource;
    delete m_Microphone;
    delete ui;
}

void Widget::initializeWindow()
{
    const QAudioDevice &defaultDeviceInfo = QMediaDevices::defaultAudioInput();
}

void Widget::initializeAudio(const QAudioDevice &deviceInfo)
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    m_Microphone = (new Microphone(format));
    m_audioSource = (new QAudioSource(deviceInfo, format));
    qDebug() <<  "buffer size: " << m_audioSource->bufferSize();
    m_Microphone->start();
}

void Widget::initializeAudioOutput(const QAudioDevice &deviceInfo)
{
    qDebug() << "initializeAudioOutput...";
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    qDebug() << "     !!!   from  INIT format: " << format.sampleRate();
    m_Speaker.reset(new Speaker());
    m_audioOutput.reset(new QAudioSink(deviceInfo, format));
}

void buildkbNotePlayList(int tonicNote)
{
    //QList<int> kbNotePlayLists;
    int kbNoteFileName = tonicNote;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 2;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 4;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 5;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 7;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 9;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 11;
    kbNotePlayLists.append(kbNoteFileName);
    kbNoteFileName = tonicNote + 12;
    kbNotePlayLists.append(kbNoteFileName);


    qDebug() << kbNotePlayLists;
}

void Widget::paintEvent(QPaintEvent * /* event */)
{
    QPixmap pix(100,60);
    pix.fill(Qt::white);
    //create a QPainter and pass a pointer to the device.
    QPainter *painter = new QPainter(&pix);
    QPen outsidePen(Qt::red, 4, Qt::SolidLine);
    painter->setPen(outsidePen);
    painter->drawEllipse(35, 15, 30, 30);
    QPen insidePen(Qt::green, 4, Qt::SolidLine);
    painter->setPen(insidePen);
    painter->drawEllipse(40 + accValue, 20, 20, 20);
    QPen linePen(Qt::black, 1, Qt::SolidLine);
    painter->setPen(linePen);
    painter->drawLine(0, 30, 100, 30);
    painter->drawLine(50, 0, 50, 60);
    painter->end();
    ui->lbTuner->setPixmap(pix);
}

void Widget::on_btnStart_clicked()
{
    m_Microphone->moveToThread(&MicThread);
    MicThread.setObjectName("MicThread");
    MicThread.start();

    restartAudioStream();
    qDebug() << "start pushed...";
    ui->btnStart->setVisible(false);
    ui->txtPlayed->setText("0");
    ui->txtGood->setText("0");
    qDebug() << "starting...";
    curLessonInt = currentlesson.toInt();
    // get sound array set
    tonicNote = tonic_map[gNote[curLessonInt-1]];
    qDebug() << tonicNote;
    qDebug() << gNote[curLessonInt-1];
    FileLoader files;
    files.GetFileList(tonicNote);
    buildkbNotePlayList(tonicNote);
    playedCnt = 0;
    goodCnt = 0;
    nPos = 0;
    m_Speaker->moveToThread(&SpeakerThread);
    do_Orientation(nPos);
    nPos++;
}

void Widget::updateKBnote(int kbValue, float acc)
{
    int letter = kbValue%12;
    int octaveValue = kbValue/12;
    QString theNote = note_letters[letter] + QString::number(octaveValue);
    qDebug() << "knNote... " << theNote;
    QString alphaNote = note_letters[letter] + QString::number(octaveValue);
    qDebug() << "alphaNote... " << alphaNote;
    ui->lbNote->setText(alphaNote);
    qDebug() << "acc is: " << (int)(acc * 1000);
    accValue = (int)(acc * 1000);
    ui->lbTuner->repaint();
}

void Widget::do_Orientation(int)
{
    kbPlayed = kbNotePlayLists[orientation[nPos] - 1];
    m_Speaker->newTest( rawRecArrays[orientation[nPos] - 1]);
    qDebug() << "orientation value: " << orientation[nPos] - 1;
    m_Speaker->stop();
    m_Speaker->start();
    m_audioOutput->stop();
    m_audioOutput->start(m_Speaker.data());
    SpeakerThread.start();
}

void Widget::do_Quiz(int)
{
    kbPlayed = kbNotePlayLists[testNotes[nPos] - 1];
    m_Speaker->newTest( rawRecArrays[testNotes[nPos] - 1]);
    qDebug() << "testNotes value: " << testNotes[nPos] - 1;
    m_Speaker->stop();
    m_Speaker->start();
    m_audioOutput->stop();
    m_audioOutput->start(m_Speaker.data());
    SpeakerThread.start();
}

void Widget::stop_mic()
{
    m_audioSource->reset();
    m_audioSource->start();
}

void Widget::TimeOut()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Timed Out", "Continue?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qDebug() << "continuing...";
    } else {
        qDebug() << "No was clicked";
        QApplication::quit();
    }
    qDebug() << "continuing...";
    for(int i = 0; i < 200000; i++)
    {
        rec_arr[i] = 0;
    }
    rec_arr_cnt = 0;
    frame_start = 0;
    frame_end = 2048;
    m_Microphone->reset();
    m_audioSource->reset();
    restartAudioStream();
    QThread::msleep(100);
    nPos--;
    do_Orientation(nPos);
    nPos++;
}

void Widget::Got_Note(int kbValue)
{
    playedCnt++;
    m_audioSource->stop();
    stopSound();
    ui->txtPlayed->setText(QString::number(playedCnt));
    if (kbValue == kbPlayed)
    {
        goodCnt++;
        ui->txtGood->setText(QString::number(goodCnt));
    }

    QThread::msleep(displayDuration);
    if(nPos < 21 and orientationFlag)
    {
        ui->btnNext->click();
    }
    if(nPos < 20 and !orientationFlag)
    {
        ui->btnNext->click();
    }
}

void Widget::stopSound()
{
    qDebug() << "test Sound...";
    SpeakerThread.exit();
    SpeakerThread.start();
}

void Widget::restartAudioStream()
{
    m_audioSource->stop();
    qDebug()<< "is main: " << QThread::isMainThread();
    m_audioSource->start(m_Microphone);
    qDebug() << "============================";
}

void Widget::on_btnStop_clicked()
{
    m_audioSource->stop();
}

void Widget::on_btnNext_clicked()
{
    qDebug() << "next pressed";
    qDebug() << "MicThread : " << MicThread.isRunning();
    qDebug() << "SpeakerThread : " << SpeakerThread.isRunning();
    qDebug() << "m_Microphone is open: " << m_Microphone->isOpen();
    // zero out rec_arr with each mic get
    for(int i = 0; i < 200000; i++)
    {
        rec_arr[i] = 0;
    }
    rec_arr_cnt = 0;
    frame_start = 0;
    frame_end = 2048;
    m_Microphone->reset();
    m_audioSource->reset();
    restartAudioStream();
    QThread::msleep(100);
    if (orientationFlag == true)
    {
        do_Orientation(nPos);
        if(nPos == 20)
        {
            MicThread.exit();
            float scorePercent = (goodCnt * 100) / playedCnt;
            ui->txtOrientationScore->setText(QString::number(scorePercent) + "%");
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Orientarion Complete", "Continue?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                qDebug() << "continuing...";
            } else {
                qDebug() << "No was clicked";
                QApplication::quit();
            }
            orientationFlag=false;
            playedCnt = 0;
            goodCnt = 0;
            nPos = 0;
            MicThread.start();
        }
    } else {
        do_Quiz(nPos);
        if(nPos == 21)
        {
            MicThread.exit();
            SpeakerThread.exit();
            float scorePercent = (goodCnt * 100) / playedCnt;
            ui->txtLessonScore->setText(QString::number(scorePercent) + "%");
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "lesson Complete", "Continue?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                qDebug() << "continuing...";
            } else {
                qDebug() << "No was clicked";
                QApplication::quit();
            }
            orientationFlag=true;
            qDebug() << "test complete";
            ui->btnNext->setVisible(false);
            nPos = 0;
        }
    }
    nPos++;
}

