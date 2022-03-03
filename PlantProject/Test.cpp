#include "Test.h"
#include <AnalizadorSegmentacionFijo.h>

using namespace cv;
using namespace std;

Test::Test()
{

}
//----------------------------------------------------------
void Test::crop_test() {
    Mat img = imread("C:/Users/milena/git/PlantProject/Media/imagen-media.png");
    imshow("original", img);
    crop_time_bar(img);
    imshow("cropped", img);
}
//----------------------------------------------------------
