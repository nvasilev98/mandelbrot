
#include <iostream>
#include <complex>
#include <stdio.h>      // BMP output uses C IO not C++
#include <unistd.h>     // for getopt
#include <pthread.h>
#include "bmp.h"        // class for creating BMP files
#include <cmath>
#include <chrono>
#define E 2.71828182845904523536

using std::complex;

typedef struct
{

    pthread_t id;
    int max_iters;
    int end_col;
    int start_row;
    //int end_row;
    int num_threads;
    int value;
    long double end_x;
    long double start_x;
    long double start_y;
    long double end_y;
    Bmp_c *image;
    int max_rows;
} thread_arg_t;

int first_param_shift(int ii)
{
    return (ii *4 %256);
}
int second_param_shift(int ii)
{
    return (ii *10) %256;
}
//*****************************************************
// Determine if a single point is in the mandelbrot set.
// Params:
//    (x,y): The complex number to make the determination on
//
// Return:
//    zero if the number is in the set
//    number of iterations to conclude it's not in the set
int ComputeMandelbrot(long double x, long double y, int max_iters)
{
    complex<long double> c(x,y), z(0,0);

    for (int ii=0; ii<max_iters; ++ii)
    {
		z = pow(z, 2)*pow(pow(E,z),2) + c;
        if (std::abs(z) >= 2.0) return max_iters - ii - 1;
    }

    return 0;
}

//**************************************************
// choose a color for a particular mandelbrot value
// Params:
//     value: value returned by ComputeMandelbrot
//     max_value: the max value returned by ComputeMandelbrot
//                note: this is max_iters
// Return: 8 bit color value to be displayed
inline int ColorizeMono(int value, int max_value)
{
    if (value == 0)
        value = 255;
    else
        value = 0;

    return value;
}

//**************************************************
// choose a color for a particular mandelbrot value
// Params:
//     value: value returned by ComputeMandelbrot
//     max_value: the max value returned by ComputeMandelbrot
//                note: this is max_iters
// Return: 8 bit color value to be displayed
inline int ColorizeScaled(int value, int max_value)
{
    value = value*255/max_value*8;
    if (value > 255) value = 255;

    return value;
}

void *ThreadFunc(void *params)
{
    long double y = 0;
    long double x = 0;

    thread_arg_t *args = static_cast<thread_arg_t*>(params);

    for (int row = args->start_row; row < args->max_rows; row+=args->num_threads)
    {
        y = args->start_y + (args->end_y - args->start_y)/args->max_rows * row;

        for (int col = 0; col < args->end_col; col++)
        {
            x = args->start_x + (args->end_x - args->start_x)/args->end_col * col;
            args->value = ComputeMandelbrot(x, y, args->max_iters);

            // colorize and set the pixel
            args->value = ColorizeScaled(args->value, args->max_iters);
            if(args->value != 0)
                //printf("%LF\n", value);
                args->image->Set_Pixel(row, col, args->value);
        }
    }
    return nullptr;
}



static const char *HELP_STRING = 
"mandelbrot <options> where <options> can be the following\n"
"   -h print this help string\n"
"   -x <value> the starting x value. Defaults to -2\n"
"   -X <value> the ending x value. Defaults to +2\n"
"   -y <value> the starting y value. Defaults to -1\n"
"   -Y <value> the ending y value. Defaults to +1\n"
"   -r <value> the number of rows in the resulting image. Default 480.\n"
"   -c <value> the number of cols in the resulting image. Default 640.\n"
"   -m <value> the max number of iterations. Default is 100.\n"
"   -t <value> the number of threads to use. Default is 1.\n"
"";

//*************************************************
// Main function to compute mandelbrot set image
// Command line args:
//     -x <value> the starting x value. Defaults to -2
//     -X <value> the ending x value. Defaults to +2
//     -y <value> the starting y value. Defaults to -2
//     -Y <value> the ending y value. Defaults to +2
//     -r <value> the number of rows in the resulting image. Default 480.
//     -c <value> the number of cols in the resulting image. Default 640.
//     -m <value> the max number of iterations. Default is 100.
//     -t <value> the number of threads to use. Default is 1.
int main(int argc, char** argv)
{
	using namespace std::chrono;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int max_iters = 256;
    int rows = 480;
    int cols = 640;
    long double start_x = -2.0;
    long double end_x = 2.0;
    long double start_y = -2.0;
    long double end_y = 2.0;

    long double value =0.0;
    int num_threads = 1; //default to 1 thread

    int opt;

    // get command line args
    while ((opt = getopt(argc, argv, "hx:X:y:Y:r:c:m:t:")) >= 0)
    {
        switch (opt)
        {
            case 'x':
                sscanf(optarg, "%Lf", &start_x);
                break;
            case 'X':
                sscanf(optarg, "%Lf", &end_x);
                break;
            case 'y':
                sscanf(optarg, "%Lf", &start_y);
                break;
            case 'Y':
                sscanf(optarg, "%Lf", &end_y);
                break;
            case 'r':
                rows = atoi(optarg);
                break;
            case 'c':
                cols = atoi(optarg);
                break;
            case 'm':
                max_iters = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
                printf(HELP_STRING);
                break;
            default:
                fprintf(stderr, HELP_STRING);
        }
    }

    // create and compute the image
    Bmp_c image(rows, cols);

    //setup thread param array
    thread_arg_t * args = new thread_arg_t[num_threads];

    //create params for number of threads
    for(int i = 0; i < num_threads; i++)
    {
        args[i].start_row = i; //start each thread one row off
        args[i].num_threads = num_threads;
        args[i].max_iters = max_iters;
        args[i].end_col = cols;
        args[i].start_x = start_x;
        args[i].end_x = end_x;
        args[i].value = value;
        args[i].start_y = start_y;
        args[i].end_y = end_y;
        args[i].image = &image;
        args[i].max_rows = rows;
        pthread_create(&args[i].id, nullptr, ThreadFunc, &args[i]);
    }

    //join 'em
    for(int i = 0; i < num_threads; i++)
    {
         pthread_join(args[i].id, nullptr);
    }

    // define the pallet
    uint32_t pallet[256];
    memset(pallet, 0, sizeof(pallet));
    for (int ii=0; ii<256; ii++)
    {
        pallet[ii] = Bmp_c::Make_Color(0, first_param_shift(ii), second_param_shift(ii));
    }

    delete[] args;
    image.Set_Pallet(pallet);

    // create and write the output
    FILE *output = fopen("zad20.bmp", "wb");

    image.Write_File(output);

    fclose(output);

    printf("Image was created\n");
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "It took me " << time_span.count() << " seconds.";

    return 0;
}


