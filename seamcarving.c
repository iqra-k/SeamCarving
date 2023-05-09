
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "seamcarving.h"
#include "c_img.h"


int edge_case(int dim, int max){
    if (dim + 1 > max){
        return dim-max;
    }
    if (dim < 0){
        return dim+max;
    }
    return dim;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    create_img(grad, im->height, im->width);
    for (int y = 0; y < im->height; y++){
        for (int x = 0; x < im->width; x++){
            int bottom_r = (int)get_pixel(im, edge_case(y + 1, im->height), x, 0);
            int top_r = (int)get_pixel(im, edge_case(y - 1, im->height), x, 0);
            int right_r = (int)get_pixel(im, y, edge_case(x + 1, im->width), 0); 
            int left_r = (int)get_pixel(im, y, edge_case(x - 1, im->width), 0);
            int bottom_g = (int)get_pixel(im, edge_case(y + 1, im->height), x, 1);
            int top_g = (int)get_pixel(im, edge_case(y - 1, im->height), x, 1);
            int right_g = (int)get_pixel(im, y, edge_case(x + 1, im->width), 1); 
            int left_g = (int)get_pixel(im, y, edge_case(x - 1, im->width), 1);
            int bottom_b = (int)get_pixel(im, edge_case(y + 1, im->height), x, 2);
            int top_b = (int)get_pixel(im, edge_case(y - 1, im->height), x, 2);
            int right_b = (int)get_pixel(im, y, edge_case(x + 1, im->width), 2); 
            int left_b = (int)get_pixel(im, y, edge_case(x - 1, im->width), 2);
            int k = (top_r - bottom_r)*(top_r - bottom_r) + (right_r - left_r)*(right_r - left_r) 
                  + (top_g - bottom_g)*(top_g - bottom_g) + (right_g - left_g)*(right_g - left_g)
                  + (top_b - bottom_b)*(top_b - bottom_b) + (right_b - left_b)*(right_b - left_b);
            uint8_t energy = (uint8_t)(sqrt((double)k)/10);
            set_pixel(*grad, y, x, energy, energy, energy);
        }
    }
}

       
void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int width = grad->width;
    int height = grad->height;
    (*best_arr) = (double *)malloc(sizeof(double) * width * height);
    for (int x = 0; x < width; x++){
        (*best_arr)[x] = get_pixel(grad, 0, x, 0);
    }
    for (int y = 1; y < grad->height; y++){
        for (int x = 0; x < grad->width; x++){
            int left;
            if (x == 0) {
                left = 0;
            } else {
                left = x - 1;
            }
            int right;
            if (x == width - 1) {
                right = x;
            } else {
                right = x + 1;
            }
            int middle = x; 
            double curr_energy = get_pixel(grad, y, x, 0);
            double min = INFINITY;
            double routes[3] = {curr_energy + (*best_arr)[(y-1)*(width)+(right)], curr_energy +
            (*best_arr)[(y-1)*(width)+(middle)], curr_energy + (*best_arr)[(y-1)*(width)+(left)]};
            for (int y = 0; y < 3; y++) {
                if (routes[y] < min) {
                    min = routes[y];
                }
            }
            (*best_arr)[y*width + x] = min;
        }
    }
}




int smallest_i(double *best, int start_col, int row, int num, int width){
    int min_index;
    if (start_col < 0) {
        min_index = 0;
    } else {
        min_index = start_col;
    }
    for (int y = start_col; y < start_col + num; y++){
        if (y < 0) {
            y++;
        }
        if (y >= width) {
            break;
        }
        if (best[row*width + y] < best[row*width + min_index]) {
            min_index = y;
        }
    }
    return min_index;
}

void recover_path(double *best, int height, int width, int **path){
    *path = malloc(sizeof(int) * height);
    int min_value = smallest_i(best, 0, height - 1, width, width);
    (*path)[height - 1] = min_value;

    for(int x = height - 2; x > -1; x--){
        min_value = smallest_i(best, min_value - 1, x, 3, width);       
        (*path)[x] = min_value;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src->height, (src->width)-1);
    int pass = 0;
    for(int y = 0; y < src ->height; y++){
        pass = 0;
        for(int x = 0; x <= src->width; x++){
            if(path[y]== x){
                pass++;
            }else{
                set_pixel(*dest, y, x-pass, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), get_pixel(src, y, x, 2));

            }
        }
    }
}


/*
int main(){
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 5; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}
*/
