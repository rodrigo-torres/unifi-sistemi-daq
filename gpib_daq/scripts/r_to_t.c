#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 *   R(T) = Rz * (1 + a * T + b * T * T) + Rs
 */

double r_to_t (double r) {

  float a = 3.90802e-3;
  float b = -5.80195e-7;
  float rs = 0.561556;
  float rz = 99.9105;
  float rn;

  rn = (r - rs)/rz;                            /* normalized resistance */
  return (-a + sqrt(a*a - 4*b*(1-rn)))/(2*b);
}

int main(int argc, char * argv[]) {
	if (argc !=2) {
		puts("Please call this function with one parameter, the resistance read");
	}

	double t = atof(argv[1]);
	printf("Temperature is %f corresponding to %f\n", r_to_t(t), t);

	return 0;
}
