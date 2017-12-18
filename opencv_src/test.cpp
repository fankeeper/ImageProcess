#include </usr/local/Cellar/opencv/3.3.1_1/include/opencv2/opencv.hpp>
#include </usr/local/Cellar/opencv/3.3.1_1/include/opencv2/highgui/highgui.hpp>
using namespace cv;

int main(int argc, char** argv)
{   
    // Load image
    Mat source_img = imread("source_img.bmp", CV_LOAD_IMAGE_UNCHANGED);

    if(source_img.empty())
    {
        fprintf(stderr, "failed to load input image\n");
        return -1;
    }

    // Gray image
    Mat gray_img;
    cvtColor( source_img, gray_img, CV_BGR2GRAY );

    if( ! imwrite("gray_img.bmp", gray_img) )
    {
        fprintf(stderr, "failed to write gray_img file\n");
    }

    // Binary image
    Mat binary_img;
    threshold( gray_img, binary_img, 300, 255, CV_THRESH_OTSU ); 

    if( ! imwrite("binary_img.bmp", binary_img) )
    {
        fprintf(stderr, "failed to write binary_img file\n");
    }

    // Canny
    Mat canny_img;
    blur( gray_img, canny_img, Size(3, 3) );
    Canny( canny_img, canny_img, 150, 100, 3 );

    if( ! imwrite("canny_img.bmp", canny_img) )
    {
        fprintf(stderr, "failed to write canny_img file\n");
    }

    // Harris
    Mat corner_strength;
    cornerHarris( gray_img, corner_strength, 3, 3, 0.01 );

    Mat harris_img;
    threshold( corner_strength, harris_img, 0.0001, 255, THRESH_BINARY_INV );

    if( ! imwrite("harris_img.bmp", harris_img) )
    {
        fprintf(stderr, "failed to write harris_img file\n");
    }

    // enlarge *2
    Mat enlarge_img;
    pyrUp( source_img, enlarge_img, Size(source_img.cols*2, source_img.rows*2) );

    if( ! imwrite("enlarge_img.bmp", enlarge_img) )
    {
        fprintf(stderr, "failed to write enlarge_img file\n");
    }

    // reduce /2
    Mat reduce_img;
    pyrDown( source_img, reduce_img, Size(source_img.cols/2, source_img.rows/2) );

    if( ! imwrite("reduce_img.bmp", reduce_img) )
    {
        fprintf(stderr, "failed to write reduce_img file\n");
    }

    return 0;
}