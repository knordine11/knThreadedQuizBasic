#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QPixmap>
#include <QPainter>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Microphone : public QIODevice
{
    Q_OBJECT

public:
    Microphone(const QAudioFormat &format);
    ~Microphone();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    qreal getNoteValue(const char *data, qint64 len) const;

signals:
    void levelChanged(qreal level);
    void on_TimeOut() const;

private:
    const QAudioFormat m_format;
    qreal m_level = 0.0; // 0.0 <= m_level <= 1.0
};

class Speaker : public QIODevice
{
    Q_OBJECT

public:
    Speaker();
    void start();
    void stop();
    void getWaveFile(QString noteFile);
    void newTest(QByteArray bufferOut);
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;
    qint64 size() const override { return m_buffer.size(); }
    qint64 m_pos = 0;
    QByteArray m_buffer;

public slots:
    void startSound();
    void stopSound();
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QThread MicThread;
    QThread SpeakerThread;
    QMediaDevices *m_devicesOut = nullptr;
    QScopedPointer<Speaker> m_Speaker;
    QScopedPointer<QAudioSink> m_audioOutput;
    bool orientationFlag;
    bool lessonDoneFlag;
    void do_Orientation(int);
    void do_Quiz(int);
    void play_next_note();
    void timeoutRetry();
    void getNextLesson();
    int tonicNote;
    int playedCnt;
    int goodCnt;
    int nPos;
    int kbPlayed;

protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:
    void stopSound();
    void updateKBnote(int kbValue, float acc);
    // void stop_mic();
    void TimeOut();
    void Got_Note(int kbValue);

private slots:
    void on_btnStart_clicked();

    void on_sldDuration_valueChanged(int value);

private:
    void initializeWindow();
    void initializeAudio(const QAudioDevice &deviceInfo);
    void initializeAudioOutput(const QAudioDevice &deviceInfo);
    void restartAudioStream();
    QMediaDevices *m_devices = nullptr;
    Microphone *m_Microphone;
    QAudioSource *m_audioSource;
    bool m_pullMode = false;
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
