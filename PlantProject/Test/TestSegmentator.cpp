#include "TestSegmentator.h"
#include <QDebug>

#include <Analyzer.h>
#include <StaticModelSegmentator.h>
#include <GlobalSegmentator.h>

using namespace cv;
using namespace std;

TestSegmentator::TestSegmentator()
{

}
//----------------------------------------------------------
void TestSegmentator::crop_test() {
    Mat img = imread("C:/Users/milena/git/PlantProject/Media/imagen-media.png");
    imshow("original", img);
    crop_time_bar(img);
    imshow("cropped", img);
}
//----------------------------------------------------------
void TestSegmentator::show_frames() {
    VideoCapture video;
    Mat frame;
    int start = 3270;
    int increment = 1;
    int end = start + 90;

    video.open("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
    video.set(CAP_PROP_POS_FRAMES, start);
    for (int i = start; i < end; i += increment) {
        if (increment != 1) video.set(CAP_PROP_POS_FRAMES, i);
        video >> frame;
        if (i == start || i == end-1) show_frame(frame, to_string(i));

        cvtColor(frame, frame, COLOR_RGB2GRAY);
        Scalar m = mean(frame);

        cout << "show_frames: " << i << " media: " << m[0] << endl;
    }
}
//----------------------------------------------------------
void TestSegmentator::variations() {
     VideoCapture video;
     Mat frame;
     vector<int> v;
     vector<double> d;
     Scalar gray;
     double dif;

     video.open("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
     cout << "start" << endl;

     double last = 0.0;
     int i = 0;

     for(;; i++) {
        video >> frame;

        if(frame.empty()) break;

        cvtColor(frame, frame, COLOR_BGR2GRAY);
        gray = mean(frame);

        if ((dif = fabs(gray[0] - last)) > 10.0) {
            v.push_back(i);
            d.push_back(dif);
        }

        last = gray[0];
     }

     for (uint j = 0; j < v.size(); j++) {
         cout << "[" << j << "]" << v[j] << ": " << d[j] << endl;
     }
     cout << "done." << endl;
}
//----------------------------------------------------------
void TestSegmentator::variations_2() {
    VideoCapture video("C:/Users/milena/git/PlantProject/Media"
                       "/climbing_bean_project3_leaf_folding.AVI");
    Mat frame;
    Scalar gray;

    size_t num = 5;
    vector<double> vec(num);
    vector<int> frames;
    vector<double> difs;
    size_t i = 0;
    double sum = 0.0;
    double mean;
    double l_mean;
    double dif;

    // INIT
    for (uint j = 0; j < num; j++) {
        video >> frame;
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        gray = cv::mean(frame);

        vec.push_back(gray[0]);
        sum += gray[0];
        qDebug() << "variations_2: gray" << gray[0]
                 << "sum =" << sum;
    }
    l_mean = sum / num;

    qDebug() << "variations_2: l_mean =" << l_mean
             << "sum =" << sum << "vec.size =" << vec.size();
    qDebug() << "variations_2: init done, alg start";

    // ALG
    for (;; i = (i+1) % num) {
        video >> frame;
        if (frame.empty()) break;
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        gray = cv::mean(frame);

        sum -= vec[i];
        vec[i] = gray[0];
        sum += vec[i];

        mean = sum / num;

        dif = fabs(mean - l_mean);
        if (dif > 1.0) {
            frames.push_back(static_cast<int>(video.get(CAP_PROP_POS_FRAMES)));
            difs.push_back(dif);
        }
        l_mean = mean;
        cout << i;
    }

    for (uint j = 0; j < difs.size(); j++) {
        cout << frames[j] << ": " << difs[j] << endl;
    }

    qDebug() << "variations_2: alg done.";
}
//----------------------------------------------------------
void TestSegmentator::breakpoint_increment() {
    int DN_BREAKPOINTS[] = { 0, 436, 1395, 1877, 2836, 3314 };
    VideoCapture video("C:/Users/milena/git/PlantProject/Media"
                       "/climbing_bean_project3_leaf_folding.AVI");
    Mat frame;

    cout << "breakpoint_increment: start POS_FRAMES =" << _video.get(CAP_PROP_POS_FRAMES);

    for (int i = 1; i < 6; i++) {
        cout << "breakpoint_increment: " << DN_BREAKPOINTS[i] - DN_BREAKPOINTS[i-1] << endl;
    }

    for (int i = 1; i < 6; i++) {
        video.set(CAP_PROP_POS_FRAMES, DN_BREAKPOINTS[i]-2);

        for (int j = 0; j < 5; j++) {
            video >> frame;
            show_frame(frame, to_string(video.get(CAP_PROP_POS_FRAMES)));

            cvtColor(frame, frame, COLOR_RGB2GRAY);
            Scalar m = mean(frame);
            cout << "breakpoint_increment: brightness = " << m[0] << endl;
        }

        cout << "--------" << endl;
    }
}
//----------------------------------------------------------
void test_outside_loop() {
   Segmentator *segm;
   Mat frame, output;
   bool mostrar;
   VideoCapture video("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
   assert(video.isOpened());

   segm = new StaticModelSegmentator();
   assert(segm != nullptr);
   qDebug() << "test: segmentator created";

   segm->set_up();
   qDebug() << "test: segmentator setup done";

   qDebug() << "test: start loop";
   for(int i = 0; ; i++) {
       video >> frame;
       if (i == 1051|| frame.empty()) break;

       mostrar = i == Segmentator::NIGHT_SAMPLE_POS || i == Segmentator::DAY_SAMPLE_POS;

       if (mostrar)
           imshow("frame" + to_string(i), frame);

       segm->process_frame(frame, output);

       if (mostrar)
           imshow("output" + to_string(i), output);
   }
   qDebug() << "test: end loop";
}
//----------------------------------------------------------
void test_analyzer() {
    Analyzer a(Analyzer::StaticModel);

    a.set_video("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
    qDebug() << "test: begin processing";
    a.process_video();
    qDebug() << "test: end processing";
}

