#include "FastSimulation/Utilities/interface/BaseNumericalRandomGenerator.h"

#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandFlat.h"

#include <cmath>
#include <iostream>

BaseNumericalRandomGenerator::BaseNumericalRandomGenerator(
			      double xmin, double xmax, int n, int iter ) :
                              xmin(xmin), xmax(xmax), n(n), iter(iter) 
{
  // Limit the array size to the hard-coded maximum 
  if ( n>1000) n=1000;
}

void
BaseNumericalRandomGenerator::initialize() {
  
  m = n-1;
  rmin = 0.;
  deltar = (double)m-rmin;

  double a[1000],y[1000],z[1000],xnew[1000];

  double sig1 = 0.;

  // Initialize sampling
  double du = (xmax-xmin)/(float)m;
  sampling[0] = xmin;
  for (int i=1; i<n; ++i) 
    sampling[i] = sampling[i-1] + du;

  // Starting point for iterations
  for (int it=0; it<iter; ++it) { 
    
    // Calculate function values
    for (int i=0; i<n; ++i )
      f[i] = function(sampling[i]);

    // Calculate bin areas
    for (int i=0; i<m; ++i )
      a[i] = (sampling[i+1]-sampling[i]) * (f[i+1]+f[i]) / 2.;

    // Calculate cumulative spectrum Y values
    y[0]=0.;
    for (int i=1; i<n; ++i )
      y[i] = y[i-1] + a[i-1];

    // Put equidistant points on y scale
    double dz = y[n-1]/(float)m;
    z[0]=0;
    for (int i=1; i<n; ++i ) 
      z[i] = z[i-1] + dz; 

    // Determine spacinf of Z points in between Y points
    // From this, determine new X values and finally replace old values
    xnew[0] = sampling[0];
    xnew[n-1] = sampling[n-1];
    int k=0; 
    for ( int i=1; i<m; ++i ) { 
      while ( y[k+1] < z[i] ) ++k;
      double r = (z[i]-y[k]) / (y[k+1]-y[k]);
      xnew[i] = sampling[k] + (sampling[k+1]-sampling[k])*r;
    }

    for ( int i=0; i<n; ++i )
      sampling[i] = xnew[i];

    sig1 = sig1 + y[m];
    //    cout << "BaseNumericalRandomGenerator::Iteration # " << it+1 
    //	 << " Integral = " << sig1/(float)(it+1) 
    //      	 << endl;
  }
}

double 
BaseNumericalRandomGenerator::generate() const {

  double r=rmin+deltar*RandFlat::shoot();
  int i=(int)r;
  double s=r-(double)i;
  //  cout << " i,r,s = " << i << " " << r << " " << s << endl;
  return sampling[i]+s*(sampling[i+1]-sampling[i]);

}

double 
BaseNumericalRandomGenerator::generateExp() const {

  double r=rmin+deltar*RandFlat::shoot();
  int i=(int)r;
  //  double s=r-(double)i;
  //  cout << " i,r,s = " << i << " " << r << " " << s << endl;
  double b = -log(f[i+1]/f[i]) / (sampling[i+1]-sampling[i]);
  double a = f[i] * exp(b*sampling[i]);

  double umin = -a/b*exp(-b*sampling[i]);
  double umax = -a/b*exp(-b*sampling[i+1]);
  double u= (umax-umin) * RandFlat::shoot() + umin;

  return -log(-b/a*u) / b;

}

double 
BaseNumericalRandomGenerator::generateLin() const {

  double r=rmin+deltar*RandFlat::shoot();
  int i=(int)r;
  //  double s=r-(double)i;
  //  cout << " i,r,s = " << i << " " << r << " " << s << endl;
  double a = (f[i+1]-f[i]) / (sampling[i+1]-sampling[i]);
  double b = f[i] - a*sampling[i];

  double umin = a*sampling[i]*sampling[i]/2. + b*sampling[i];
  double umax = a*sampling[i+1]*sampling[i+1]/2. + b*sampling[i+1];
  double u= (umax-umin) * RandFlat::shoot() + umin;

  return (-b+sqrt(b*b+2.*a*u))/a;

}


bool BaseNumericalRandomGenerator::setSubInterval(double x1,double x2){
  if(x1<xmin||x2>xmax) return false;
  if(x1>x2)
    {
      double tmp=x1;
      x1=x2;
      x2=tmp;
    }
  
  unsigned ibin=1;
  for(;ibin<(unsigned)n&&x1>sampling[ibin];++ibin);
  unsigned ibin1=ibin;
  for(;ibin<(unsigned)n&&x2>sampling[ibin];++ibin);
  unsigned ibin2=ibin;

  //  cout << sampling[ibin1-1] << " " << x1 << " " << sampling[ibin1] << endl;
  // cout << sampling[ibin2-1] << " " << x2 << " " << sampling[ibin2] << endl;

  rmin = ibin1+(x1-sampling[ibin1])/(sampling[ibin1]-sampling[ibin1-1]);
  deltar= ibin2+(x2-sampling[ibin2])/(sampling[ibin2]-sampling[ibin2-1]) - rmin;
  //  cout << rmin << " " << deltar << endl;
  return true;
}
