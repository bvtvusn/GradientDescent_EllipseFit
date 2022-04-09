// Fit an ellipse to a set of points. The ellipse cannot be rotated.

// How it works:
// first a circle is fitted to the points.
// the center of the found circle is subtracted from all the points to make the points centered around origo.
// the radius is also scaled so the average distance from origo to a point is 1. (divide by r).
// A gradient descent method is used to gradually tweak the x and y scale parameters so that the points fits optimally on the unit circle (in a least squares sense).
// Finally the xscale * r and yscale * r will tell you the width and the height of the ellipse.

// Circle fit theory here: https://goodcalculators.com/best-fit-circle-least-squares-calculator/

#include <BasicLinearAlgebra.h>
using namespace BLA;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }

  float x[] = { -23.64, -42.46, -57.82, -67.58, -72.30, -69.81, -61.08, -46.77, -27.86, -5.32, 18.82, 44.44, 67.79, 87.73, 101.97, 110.61, 112.35, 107.03, 94.89, 77.79, 55.47, 31.49, 6.92, -16.54, -37.98, -55.29, -66.02, -70.84, -69.32, -61.70, -47.76, -29.56, -6.27, 18.64};
  float y[] = { -80.85, -68.10, -50.35, -29.81, -7.20, 16.83, 37.00, 56.03, 70.89, 79.67, 82.31, 78.05, 69.68, 53.80, 34.68, 12.14, -11.34, -34.50, -55.39, -72.46, -84.83, -91.66, -90.12, -84.14, -71.01, -53.60, -32.24, -8.78, 14.51, 35.93, 54.71, 69.64, 78.69, 81.79};

  int numberOfPoints = 34;
  for ( int i = 0; i < numberOfPoints; i++) {
    Serial.print(x[i]);
    Serial.print('\t');
    Serial.println(y[i]);
  }

  Serial.println("The above points make up a circle (not evenly distributed).");



  // Fit a circle to the points:
  float circ_x;
  float circ_y;
  float circ_r;
  FindCircleCenter(x, y, numberOfPoints, circ_x, circ_y, circ_r);

  Serial.println("");
  Serial.println("Circle that fits the data best: ");

  Serial.print("X coord: ");
  Serial.println(circ_x);
  Serial.print("Y coord: ");
  Serial.println(circ_y);
  Serial.print("Radius: ");
  Serial.println(circ_r);
  Serial.println();

  // transform to centered "unit ellipse".
  float cent_x[numberOfPoints];
  float cent_y[numberOfPoints];
  for (int i = 0; i < numberOfPoints; i++) {
    cent_x[i] = (x[i] - circ_x) / circ_r;
    cent_y[i] = (y[i] - circ_y) / circ_r;
  }

// Find ellipse scale factors
  float xscale;
  float yscale;
  FindEllipsefactors(cent_x, cent_y, numberOfPoints, xscale, yscale);
  Serial.println(xscale, 5);
  Serial.println(yscale, 5);
  Serial.println("xy scale (over)");
  // Expected scale values for the test data : 1.02647     0.96884
}

void loop() {

}

void FindEllipsefactors(float data_x[], float data_y[], int num, float& xscale_out, float& yscale_out) {
  float a = 1;
  float b = 1;
  double learningRate = 0.02; // learning rate of 0.02 confirmed stable with oval shape (y = 2*x). (with average r = 1)
  for (int i = 0; i < 50; i++)
  {
    GD_stepForward(data_x, data_y, num, learningRate, a, b);
  }
  xscale_out = 1.0 / a;
  yscale_out = 1.0 / b;
}
void plotRadius(float x_co[], float y_co[], int len, float x_scale, float y_scale) {
  for (int i = 0; i < len; i++)
  {
    float magni = x_scale * x_scale * x_co[i] * x_co[i] + y_scale * y_scale * y_co[i] * y_co[i];
    Serial.println(magni, 5);
  }

}
float EllipseCost(float points_x[], float points_y[], int num_p, float a, float b) {
  // Assumes a "unit ellipse" (roughly scaled to average radius of 1).
  float sum = 0;
  float avg_scale = 0;
  for (int i = 0; i < num_p; i++)
  {
    avg_scale = 1.0 / (i + 1.0);
    sum += avg_scale * ((a * points_x[i] * a * points_x[i] + b * points_y[i] * b * points_y[i] - 1) * (a * points_x[i] * a * points_x[i] + b * points_y[i] * b * points_y[i] - 1));
  }
  return sum; // the sum is actually average cost for each point.
}
// GD variables
float f = 0;
float smallstep = 0.000001;
float f_a = 0;
float f_b = 0;
float df_da = 0;
float df_db = 0;
// </GD variables

void GD_stepForward(float points_x[], float points_y[], int num_p, float learningrate, float& a, float& b) {
  f = EllipseCost(points_x, points_y, num_p, a, b);

  f_a = EllipseCost(points_x, points_y, num_p, a + smallstep, b);
  f_b = EllipseCost(points_x, points_y, num_p, a, b + smallstep);

  df_da = (f_a - f) / smallstep;
  df_db = (f_b - f) / smallstep;
  //Serial.println("cost: ");
  //Serial.println(f, 5);
  a -= df_da * learningrate;
  b -= df_db * learningrate;
}

void FindCircleCenter(float x[], float y[], int count, float& out_x, float& out_y, float& out_r) {

  float sum_xx = 0;
  float sum_xy = 0;
  float sum_x = 0;
  float sum_yy = 0;
  float sum_y = 0;

  float sum_x_xx_yy = 0;
  float sum_y_xx_yy = 0;
  float sum_xx_yy = 0;

  for ( int i = 0; i < count; i++) {
    sum_xx = sum_xx + x[i] * x[i];
    sum_xy = sum_xy + x[i] * y[i];
    sum_x = sum_x + x[i];
    sum_yy = sum_yy + y[i] * y[i];
    sum_y = sum_y + y[i];

    sum_x_xx_yy = sum_x_xx_yy + x[i] * (x[i] * x[i] + y[i] * y[i]);
    sum_y_xx_yy = sum_y_xx_yy + y[i] * (x[i] * x[i] + y[i] * y[i]);
    sum_xx_yy = sum_xx_yy + x[i] * x[i] + y[i] * y[i];
  }

  BLA::Matrix<3, 3> lhs = {sum_xx, sum_xy, sum_x, sum_xy, sum_yy, sum_y, sum_x, sum_y, count};
  BLA::Matrix<3, 1> rhs = {sum_x_xx_yy, sum_y_xx_yy, sum_xx_yy};
  Invert(lhs);
  BLA::Matrix<3, 1> result = lhs * rhs;
  float a = result(0); // coeff
  float b = result(1); // coeff
  float c = result(2); // coeff

  out_x = a / 2; // x coordinate of center
  out_y = b / 2; // y coordinate of center
  out_r = (sqrt(4 * c + a * a + b * b)) / 2; // Radius of circle
}
