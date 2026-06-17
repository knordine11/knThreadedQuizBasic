#include "fftstuff.h"
#include <iostream>
#include "fftw3/fftw3.h"
#include "qdebug.h"
#include <cmath>
#include <iomanip>
#include <QTimer>

using namespace std;
bool collectMicData = true;

bool is_tuner = true;
int noteA_oct;
int noteA_no;   // ans A base
float note_acc;
int noteC_oct;  // ans C base
int noteC_no;

int nnAns;
int nnQues;
int nnDiff;
int nnOctDiff;


//  = 2^(X/12)
float nn_l[] = {1.000000, 1.0594630944, 1.1224620483, 1.1892071150,
                1.25992104991, 1.3348398542, 1.4142135624, 1.4983070769,
                1.5874010520, 1.6817928305, 1.7817974363, 1.8877486254};

double temp_freq = 0;   // temp used for testing only
double rec_arr[1000000];
// float rec_arr_end =100000;
int rec_arr_end =200000;
int rec_arr_cnt = 0;
int frame_start = 0;
int frame_size = 2048;
int frame_end = frame_start + frame_size;
double binA1;
double binA2;
double binA3;
double binA4;
double binA5;
double binA3floor = 10;
int binNo1;
int binNo2;
int binNo3;
double freq1;
double freq2;
double freq3;
double bin_amp[3];
int bin_no[3];

double l;
double h;
double hl;
double hl_int;
double hl_rem;
double hl_rem_a;
double hl_rem_a_abs;
double hl_l;
double hl_h;

double Fs = 44100;
const int N = 16384*4; // 2^14 * 4 =  2^16 = 65,536
double bin_size = Fs/N;

fftw_complex* in, * out;

double last_peak = 0;
double peak_val = 0;
double peak_f_bin = 0;
double peak_diff = 0;
double lev1 = 0;
double lev2 = 0;
double lev3 = 0;
double TlevL = 0;
double TlevH = 0;
double levA = 0;
double move_by = 0;
double freq_got =0;
bool Flag_up = true;

int nn_compair=-1;
int nn_compair_cnt = 0;

FftStuff::FftStuff(QObject *parent)
    : QObject{parent}
{

}

// ================         THE MAIN MODUAL STARTS         ============================

void FftStuff::DoIt(int beg, int lengh)
{

    int at = beg;
    int end_at = beg+lengh;
    cout<<"  FftStuff::DoIt  " <<beg<<"         "<<lengh<<  "         "<< end_at<<endl;
    fftw_plan p;
    fftw_free(in);
    fftw_free(out);

    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    for (int i = 0; i < N; i++)
    {
        if(i<end_at){
            in[i][0] = rec_arr[at];
            at++;
        }else {
            in[i][0] = 0.0;}
        in[i][1] = 0.0;
    }
    fftw_execute(p); /* repeat as needed */
    std::cout << std::fixed;
    std::cout << std::setprecision(4);

    // =======      PROSSES FFT OUTPUT       FIND THE HIGHEST BIN FROM FFT OUTPUT

    clear_highest_peaks_arr();  //zero highest bins and prosses fft output
    for (size_t i = 0; i < N; i++)
    {
        if(i<6000){
            // double cur_bin = i * Fs/N;
            // std::cout<<i<<"  " << cur_bin << " Hz : " << abs_c(out[i]) << std::endl;
        }
    }
    // PROSSES FFT OUT PUT   AND GET TRUE PEAKS
    // cout << endl << "  got here    PROSSES FFT OUT PUT   AND GET TRUE PEAKS "<<endl;
    double last_lev = 0;
    int last_peak = 0;
    double last_peak_val =0;
    bool up = true;
    bool peak_up = true;
    for(int i = 20; i<6000; i++){
        double val_out = abs_c(out[i]);
        // cout << i <<" ll "<< last_lev<<" vo "<<val_out <<"  up "<<up<<"  pup  "<<peak_up<<endl;
        if(up)
        {
            if(last_lev > val_out)          // peak found @ i-1
            {
                int peak = i-1;
                // cout<<"                    FOUND PEAK AT  BIN  "<< peak <<"  :  " << last_lev << endl;
                if(peak_up && last_peak_val>last_lev){
                    // cout <<endl<< "                 ----------    TRUE PEAK AT  { BIN "<< last_peak
                    // << " }  level "<< last_peak_val<<endl;
                    save_highest_bin_peaks(last_peak,last_peak_val);
                    bin_to_freq(last_peak);
                }
                if(last_peak_val > last_lev){
                    peak_up = false;
                }else{ peak_up = true;}
                last_peak_val = last_lev;
                last_peak = peak;
                up = false;
            }
        }
        // cout <<"** ";
        if(val_out>last_lev){up=true;}
        last_lev = val_out;
    }

    // HAVE THE THREE HIGHEST PEAKS     now find the fundenatal frequency
    // fftw_destroy_plan(p); // SAVED FOR DONE NOW EXIT
    // ===================     GET FUNDEMENTAL FREQUENCY   ========================

    double fun_freq = FftStuff::get_fun();
    if(fun_freq < 26){
        nn_compair = -1;        // NOT GOOD NOTE
        nn_compair_cnt= 0;      // RESET FRAME FILTERING
        return;}                // EXIT

    Note note;
    note.freq_to_note(fun_freq);    // ans are globle

    // ================  HAVE NOTE  ====  NOW DO FRANE FILTERING  ==================

    if(noteC_no != nn_compair) {   nn_compair = noteC_no; nn_compair_cnt = 0;}
    else {    nn_compair_cnt++; }
    // qDebug()<< "     ----          nn_compair_cnt "<< nn_compair_cnt << "     nnAns " <<nnAns;
    if(nn_compair_cnt < 6)
    {
        return;
    }
    else
    {
        int kbValue = noteC_no + noteC_oct*12;
        emit valueChanged(kbValue, note_acc);
        qDebug()<< ">>>>>>oct value = " << noteC_oct;
        qDebug()<< ">>>>>>note value = " << noteC_no;
        qDebug()<< ">>>>>>note acc = " << note_acc;
        qDebug()<< ">>>>>>kbValue = " << kbValue;
        emit on_foundNote(kbValue);

    }

    //  ======= PASSED FILTERING  =====   NOW  PROCESSES A GOOD FILTERED NOTE =======

    qDebug()<< "\n\n--- %%%% ---  $$$$  ---  HAVE GOOD FILTERED NOTE  ---  $$$$  ---  %%%%\n ";
    collectMicData = false;  // NO MORE MIC DATA needed STOP IT
    nnAns = noteC_no;

    // Arrow pos   based on 1st pos = 0   is nnAns
    nnDiff = nnQues - nnAns;

    // Tile no = nnQues
    // nnOctDiff = octTonic - noteC_oct

    qDebug() << "===%%%%%%%%%%%%%%%   before timer %%%%%%%%%%%%";

    qDebug()<< "note value = " << noteC_no;
    rec_arr_cnt = 0;

    qDebug() << "===%%%%%%%%%%%%%%%   after timer %%%%%%%%%%%%";

}
// -------------------   end of main   modual   -------------------------------------------

void FftStuff::onTimeout()
{
    qDebug() << "   $$$$$ %%%%%  TIME OUT HAPPENED  ";
}

double FftStuff::abs_c(fftw_complex c)
{
    return std::sqrt(c[0] * c[0] + c[1] * c[1]);
}


void FftStuff::save_highest_bin_peaks(int bin, double bin_amp)
{
    //    cout<< "FROM save_highest_bin_peaks         [ "<< binNo1<<" ]  "<<binA1
    //            <<"     [ "<< binNo2<<" ]  "<<binA2
    //           <<"     [ "<< binNo3<<" ]  "<<binA3<<"     LAST levals                     "<<binA4<<"   "<<binA5 <<endl;
    if(bin_amp > binA1){
        binA5 = binA4;  binA4 = binA3;   binA3 = binA2;   binA2 = binA1;   binA1 = bin_amp;
        binNo3 = binNo2; binNo2= binNo1;  binNo1= bin; return;}
    if(bin_amp > binA2){
        binA5 = binA4;  binA4 = binA3;   binA3 = binA2;   binA2 = bin_amp;
        binNo3 = binNo2; binNo2= bin;   return;}
    if(bin_amp > binA3){
        binA5 = binA4;  binA4 = binA3;  binA3 = bin_amp;
        binNo3= bin;  return;}
    if(bin_amp > binA4) {binA5 = binA4; binA4 = bin_amp; return;}
    if(bin_amp > binA5) {binA5 = bin_amp;return;}
}

void FftStuff::clear_highest_peaks_arr()
{
    binA1 = 0;  binA2 = 0;  binA3 = 0; binA4 = 0; binA5 = 0;
    binNo1= 0;  binNo2= 0;  binNo3= 0;
}

double FftStuff::bin_to_freq(int bin){
    double bin_freq = bin*bin_size;
    double lev_l = abs_c(out[bin-1]);
    double lev_ = abs_c(out[bin]);
    double lev_h = abs_c(out[bin+1]);

    double bin_l = bin-1;
    double bin_ = bin;
    double bin_h = bin+1;

    double bin_la = bin_l/bin_;
    double bin_ha = bin_h/bin_;

    double bin_lb = 1-bin_la;
    double bin_hb = 1-bin_ha;

    double bin_lc = bin_lb/2;
    double bin_hc = bin_hb/2;

    double bin_ld = bin_lc+1;
    double bin_hd = bin_hc+1;

    double lev_la = lev_l * bin_ld;
    double lev_ha = lev_h * bin_hd;

    double lev_lt = lev_-lev_l;
    double lev_ht = lev_-lev_h;

    // cout <<endl<< bin-1 << " bin_la " << bin_la << "  bin_lb " <<bin_lb<< "  bin_lc " <<bin_lc
    //      << "  bin_ld " <<bin_ld
    //      << "    lev_l "<<lev_l << "  | lev_la " << lev_la
    //      << "  |  lev_lt " <<lev_lt         <<endl;
    // cout << bin+1 << " bin_ha " << bin_ha << "  bin_hb " <<bin_hb<< "  bin_hc " <<bin_hc
    //      << "  bin_hd " <<bin_hd
    //      << "    lev_h "<<lev_h << "  | lev_ha " << lev_ha
    //      << "   | lev_ht " <<lev_ht         <<endl;

    double diff = lev_lt - lev_ht;
    double r = 0;   // used so r could be used in (if else) code

    if(lev_lt < lev_ht){
        double diff = lev_lt - lev_ht;
        r = diff / lev_ht;
    }else{
        r = diff / lev_lt;
    }
    double rh = r/2;
    double move = rh * bin_size;
    double freq = bin_freq + move;
    double p_diff = ((freq/temp_freq)-1)* 100;
    //    cout << "  diff = "<< diff <<"   r = "<< r << "   r/2 "<<rh <<
    //        "   move " << move<<endl<<
    //       "   freq in = "<<temp_freq<<"      freq got = "<< freq <<"      % diff = "<<p_diff<<" % "<<endl<<endl;
    return freq;
}


double FftStuff::get_fun()
{
    // double FftStuff::get_fund_freq(){
    double freq1 = bin_to_freq(binNo1);
    double freq2 = bin_to_freq(binNo2);
    double freq3 = bin_to_freq(binNo3);
    double tol_floor = 6;
    double tol_max_peak = 20;
    double tol_3rd_peak = 10;
    int har_l;
    int har_h;
    int har_ = 0;
    double fun_freq_1;
    double fun_freq_2;
    double fun_freq_1_2;
    double diff_f;
    double diff_a;
    double move_f;
    // cout<<endl<<"---------------------------------------   GET FUN  -----------------"<<endl;
    // cout << " 1 [ " << freq1<<" ]         lev "<<binA1<<endl<<
    //     " 2 [ " << freq2<<" ]         lev "<<binA2<<endl<<
    //     " 3 [ " << freq3<<" ]         lev "<<binA3<<
    //     " --------- last lev 4 , 5 ->  "<<binA4<<"   "<<binA5<<endl<<endl;

    if(binA1 < tol_max_peak){cout<<"          # (1)   1st pk freq   TOO LOW    <-----  NO GOOD"<<endl<<endl;return 0.0;};
    if(binA2 < tol_floor){cout<<"          # (2)   2nd freq TOO LOW  <----- FIRST FREQ"<<endl;return freq1;};

    l = freq1;  h = freq2;
    if(h<l){l = freq2; h = freq1;}; // set up for varables for test
    har_l = harnonic(l,h);
    har_h = hl_int;
    if(har_l == 0){return 0;}               // NO HARMONICS FOUND
    if(har_l == 1 && binA3 > tol_3rd_peak)  // CK FOR LOWEST FREQ NOT 1ST BUT 2ND FREQ ( OCT DIFF )
    {
        //cout<<"                                  {  CK 1st freq  }"  <<endl;
        if(freq3 < l){
            har_ = harnonic(freq3,h);
            if(har_ == 1){
                har_l = 2;
                har_h = har_h * 2;
            }
            // cout<<"             <  LESS THAN   freq3 < l      har_l = "<<har_l<<endl;
        }
        else{
            har_ = harnonic(l,freq3);
            if(har_ != 0){
                har_h = har_h * har_;}
            // cout<<"             >  GRATER THAN  l,freq3        har_l = "<<har_l<<endl;
        }
        // cout<<"                  har_l = "<<har_l<<"    har_h = "<<har_h<<endl<<endl;
    }
    // HAVE har_l and har_h  NOW weight ave the two  ---------------------------------
    if(freq1 == l){
        fun_freq_1 = freq1 / har_l;
        fun_freq_2 = freq2 / har_h;
        // cout<<"  FROM  IF   ";
    }
    else{
        fun_freq_1 = freq1 / har_h;
        fun_freq_2 = freq2 / har_l;
        // cout<<" FROM  ELSE       ";
    }
    // cout<<" fun_freq_1 =  "<<fun_freq_1<<"    fun_freq_2 =  "<<fun_freq_2<<endl;
    diff_f = fun_freq_2 - fun_freq_1;
    diff_a = binA2/binA1;
    move_f= diff_f * diff_a/2;
    fun_freq_1_2 = fun_freq_1 + move_f;
    // cout<<endl<<"diff_f =  "<<diff_f<<"   diff_a =  "<<diff_a
    //      <<"   move_f =  "<< move_f  <<"    <<      fun_freq_1_2 =   "<<fun_freq_1_2<<endl;
    return fun_freq_1_2;
}


double FftStuff::harnonic(double l, double h)
{
    double x = 1;
    for(int i= 1; i < 4; i++){
        x = i;
        hl = h/l*x;
        hl_int = round(hl);
        hl_rem = (hl_int - hl);
        hl_rem_a = hl_rem/hl_int;
        hl_rem_a_abs = abs(hl_rem_a);
        cout<<" "<<i<<"  hl "<<hl<<"    hl_int "<<  hl_int<<"   hl_rem "<<  hl_rem
             <<"    hl_rem_a "<<  hl_rem_a<< "     hl_rem_a_abs    =  "<<hl_rem_a_abs  <<endl;
        if(hl_rem_a_abs < .014){ return i;}  // return x harmonis number of l frequency
    }
    //cout<<"\n  -----       RETURING 0.0 BECAUSE NO HARMONIC FOUND \n";
    return 0;   // NO HARMONIC FOUND
}


void Note::freq_to_note(float freq)
{
    cout<<endl;
    float base = 27.5;
    int oct_a = 1;
    int note_a_no  = 12;
    float b_line_const = 0.9715319412;  //  2^(-1/24) BOARDER HALF WAY BETWEEN NOTES
    float botton_line = base*2 * b_line_const;
    // cout<<"[ f_to_n ]      init  DEFIND   --->       botton_line       "  << botton_line <<endl;
    // float botton_line =53.854;  // (((2 * base - 1.917 *base)/2) -(2 * base)
    while(freq > botton_line){
        oct_a ++;
        botton_line = botton_line * 2;
        base = base * 2;
        // cout<<oct_a<<"  botton_line "  <<botton_line<< "    space_diff "<<space_diff<<endl;
    }
    // cout<<oct_a<<" [ oct ]     EXIT  base "<<base<<"  space_diff "<<space_diff<<endl;
    noteA_oct = oct_a;
    // cout<<"  ----------------   GOT OCT A = "<<oct_a<<endl<<endl;

    while(freq < botton_line){
        // cout<<" note_a_no = " << note_a_no <<"   botton_line "<<botton_line<<endl;
        note_a_no --;
        botton_line =nn_l[note_a_no] * base * b_line_const;
    }
    noteA_no = note_a_no;
    float note_pit = base * nn_l[note_a_no] ;
    // cout<<"   ----------------------------------- HAVE NOTE"<<endl;
    // cout<<" note_a_no = " << note_a_no<<"   botton_line "<<botton_line
    // <<"          freq "<<freq<<"   note_pit "<<note_pit<<endl<<endl;
    note_acc = ((freq/note_pit) -1) /** 1000*/;
    // cout<< "noteA_oct = "<<noteA_oct<<"    noteA_no = "<<noteA_no<<"       note_pit "<<note_pit<<"   noteA_acc "<<note_acc<<endl<<endl;

    //cout<<" CONVERT FROM A BASE TO C BASED     ";
    noteC_no = noteA_no -3;
    noteC_oct = noteA_oct;

    if(noteC_no<0){
        noteC_no = noteC_no + 12;
        noteC_oct --;}

    cout <<"  ( ( C base) -->  [ "
         <<noteC_oct<<" "<<noteC_no<< " ]   acc = "<<note_acc<< endl<< endl;
}
