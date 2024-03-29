#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int main(int argc, char** argv )
{
    //argument check
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    //image reading
    Mat image;
    image = imread( argv[1], 1 );
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    //Showing image
    /*
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    waitKey(0);
    */

    //channels and rows and columns
    int nChannels=image.channels();
    int nRows=image.rows;
    int nCols=image.cols;
    //cout<<"rows="<<nRows<<" cols="<<nCols<<" channels="<<nChannels<<endl;
    
    long entropy=0,fx,fy;
    for( int y = 1; y < image.rows-1; y++ )
    { for( int x = 1; x < image.cols-1; x++ )
         { for( int c = 0; c < nChannels; c++ )
              {
                fx=(2*(long)image.at<Vec3b>(y,x)[c]-(long)image.at<Vec3b>(y,x-1)[c]-(long)image.at<Vec3b>(y,x+1)[c]);
                fy=(2*(long)image.at<Vec3b>(y,x)[c]-(long)image.at<Vec3b>(y-1,x)[c]-(long)image.at<Vec3b>(y+1,x)[c]);
                //cout<<temp<<endl;
                entropy=entropy+fx*fx+fy*fy;
                        //cout<<(int)image.at<Vec3b>(y,x)[c]<<endl;
                        //image2[y][x][c]=(int)image.at<Vec3b>(y,x)[c];          
              }
         }
    }
    //cout<<entropy<<endl;         
    float avg_entropy=(float)entropy/(nRows*nCols);
    cout<<argv[1]<<" entropy="<<avg_entropy<<endl;         
    return 0;
}
