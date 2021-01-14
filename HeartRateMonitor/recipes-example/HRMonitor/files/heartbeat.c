#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <errno.h>

#define q	11	 	/* for 2^11 points */
#define N	(1<<q)		/* N-point FFT, iFFT */

typedef float real;
typedef struct{real Re; real Im;} complex;
complex v[N]; // array that contains the values obtained from sensor

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif

void fft( complex *v, int n, complex *tmp);
void *read_value(void *arg);
void *thread_bpm() ;
void bpm (complex *v);

// need to define a struct to pass different arguments to a thread
struct thread_args { 
  int file;
  int index;
  int value;
};

int main(int argc, char **argv) {
	
  char *app_name = argv[0];
  char *dev_name = "/dev/heartbeatmodule";
	
  int fd = -1;
  int i = 0;

  pthread_t read_values_thread[N];
  pthread_t compute_bpm;

  // Open the device
  if((fd = open(dev_name, O_RDWR)) < 0) {
	fprintf(stderr, "%s: unable to open %s: %s.\n", app_name, dev_name, strerror(errno));
	return (1);
  }

  struct thread_args *read_args = malloc(sizeof(struct thread_args));
  read_args->file = fd;
  read_args->value = 0;

  // infinite loop 
  while (1) {

  	// call a thread for each element (2048) - sample PPG sensor
	for(i=0; i<N; i++) {
		read_args->index = i;
		assert(pthread_create (&read_values_thread[i], NULL, read_value, read_args) == 0);
		pthread_detach(read_values_thread[i]);
		usleep(15000);
	}

	//pthread to perform a 2048 points fft & prints bps
	assert(pthread_create(&compute_bpm, NULL, thread_bpm, NULL) == 0);
	pthread_detach(compute_bpm);
  }

  close(fd);
  exit(EXIT_SUCCESS);
}

void fft( complex *v, int n, complex *tmp ) {
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

void *read_value(void *arg) {
	
	struct thread_args *args = (struct thread_args *) arg;
	
	read(args->file, &args->value, sizeof(int));

	// save value in the complex array
	v[args->index].Re = args->value;
	v[args->index].Im = 0;

	printf("Value: %f %f \n", v[args->index].Re, v[args->index].Im);
	pthread_exit(NULL);
}

void *thread_bpm() {
	
	int i;
	complex array[N];
	for (i=0; i<N; i++) {
		array[i].Re = v[i].Re;
		array[i].Im = v[i].Im;
	}

	bpm(array);
	pthread_exit(NULL);

}

void bpm (complex *v) {
	complex scratch[N];
 	float abs[N];
  	int k;
  	int m;
  	int i;
  	int minIdx, maxIdx;

 
	// FFT computation
  	fft( v, N, scratch );

	// PSD computation
  	for(k=0; k<N; k++) {
		abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im)); 
  	}

  	minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
  	maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm

	// Find the peak in the PSD from 30 bpm to 180 bpm
  	m = minIdx;
  	for(k=minIdx; k<(maxIdx); k++) {
    		if( abs[k] > abs[m] )
			m = k;
  	}
    
	// Print the heart beat in bpm
  	printf( "\n\n\n%d bpm\n\n\n", (m)*60*50/2048 );

}

