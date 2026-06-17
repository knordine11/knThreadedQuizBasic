#ifndef FFTSTUFF_H
#define FFTSTUFF_H

#include <QObject>
#include "fftw3/fftw3.h"
#include <cmath>
#include <QTimer>
extern double rec_arr[];
extern int rec_arr_cnt;
extern int arr_size;

using namespace std;
extern int noteA_oct;
extern int noteA_no;

// extern float noteA_acc;
extern float note_acc;
extern int noteC_no;
extern int noteC_oct;
extern float nn_l[12];
extern bool collectMicData;


class FftStuff : public QObject
{
    Q_OBJECT
public slots:
    void onTimeout();

signals:
    void valueChanged(int value, float acc);
    void on_foundNote(int);

public:

    explicit FftStuff(QObject *parent = nullptr);
    // void DoIt();
    void DoIt(int beg, int lengh);
    void DoIt2(double freq);
    static double abs_c(fftw_complex);
    static double bin_freq(size_t, size_t, double);

    void next_frame();
    void save_highest_bin_peaks(int bin, double bin_amp);
    void clear_highest_peaks_arr();

    double bin_to_freq(int bin);
    double get_fund_freq();

    double get_fun();
    double harnonic(double freq_l, double freq_h);


    void make_sin(double freq ,int beg, int leng);
    void look_rec_arr(int beg, int end);
    void graph_rec_arr(int beg, int end);

};

class Note
{
public:
    Note(){
        // cout<<" in header   THE CONSTRUCTER"<<endl;
        // fill_note_l();
    };

    float note_l[12];
    void fill_note_l();
    // void fill_note_l()
    void f_to_n(double freq);
    void freq_to_note(float freq);

    // void harmonic(double freq_l, double freq_h);

};


#endif // FFTSTUFF_H
