#include "bitmap_image.hpp"

using namespace std;


int main(){

    bitmap_image image(500,300);

    for(int i=0;i<200;i++){
        for(int j=0;j<100;j++){
            image.set_pixel(i,j,255,255,0);
        }
    }

    image.save_image("test.bmp");;

    return 0;
}
