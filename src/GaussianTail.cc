#include "FastSimulation/Utilities/interface/GaussianTail.h"

// CLHEP headers
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussQ.h"

GaussianTail::GaussianTail(double sigma,double threshold):sigma_(sigma),threshold_(threshold)
{
  s_=threshold_/sigma_;
  ssquare_ = s_ * s_;
}

GaussianTail::~GaussianTail()
{
  ;
}

double GaussianTail::shoot()
{
  // in the zero suppresion case, s is usually >2 
  if(s_>1.)
    {
      /* Use the "supertail" deviates from the last two steps
       * of Marsaglia's rectangle-wedge-tail method, as described
       * in Knuth, v2, 3rd ed, pp 123-128.  (See also exercise 11, p139,
       * and the solution, p586.)
       */
      
      double u, v, x;
      
      do
        {
          u = RandFlat::shoot();
          do
            {
              v = RandFlat::shoot();
            }
          while (v == 0.0);
          x = sqrt (ssquare_ - 2 * log (v));
        }
      while (x * u > s_);
      return x * sigma_;
    }
  else
    {
       /* For small s, use a direct rejection method. The limit s < 1
         can be adjusted to optimise the overall efficiency 
       */
      
      double x;

      do
        {
          x = RandGaussQ::shoot();
        }
      while (x < s_);
      return x * sigma_; 
    }
}
