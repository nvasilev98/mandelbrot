# Multi-Threaded Concurrent Mandelbrot Set Image Creator
This is a C++11 program created in the Linux environment that creates the Mandelbrot set as a bmp image using multiple threads concurrently.
<p>It allows you to specify the area of the Mandelbrot set to generate as well as the resolution for the bmp image and the number of threads to run on. </p>
<p>It divides the image into rows, allowing each thread to generate a row concurrently and combines the results into a single bmp. </p>

<h2>Sample Generated Image</h2>
<img src="https://github.com/levipomeroy/Mandelbrot/blob/master/6000%20x%206000%20mandelbrot.bmp" width="500" height="500"/>

<b/>
<h2>Thread Scalability</h2>
<p>This graph represents the time in seconds it takes to generate a 1000 x 1000 image of the Mandelbrot set with 1, 2, 4 and 8 threads. It depicts an increasing and nearly linear improvement in performance for additional threads, showing that the concurrency is working correctly. </p>
<img src="https://github.com/levipomeroy/Mandelbrot/blob/master/Scalability.PNG?raw=true" height="350" width="500" />
