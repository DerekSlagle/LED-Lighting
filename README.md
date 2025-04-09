# LED-Lighting
Display images or procedural patterns on an array of LED lights.
The test levels are:

** lvl_Lights **
Here all player types are being tested together. These are:
1. PulsePlayer at top of window. 2 instances are in use. When the base layer green pulse
bounces at the left end the 2nd red pulse as overlay is triggered to scroll left.
Several color blending interpolation methods ( linear, quadratic, cubic ) are in use.

2. WavePlayer is playing to the 16x16 light array on the left. It's 2 Light colors hiLt and loLt
3. are interpolated using the amplitude of the resultant sine wave. There are both left and right traveling waves.
4. The proportions are AmpRt (range 0 to 1) and AmpLt = 1 - AmpRt;
5. The default includes 1 term right going: AmpRt*sinf( k*x - f*t )
6. and 1 left going: AmpLt*sinf( k*x + f*t )
7. Harmonics may be added for either by providing a float Coeff[]. Use the function
8. void setSeriesCoeffs( float* C_rt, unsigned int n_TermsRt, float* C_lt, unsigned int n_TermsLt );

9. 3. LightPlayer2 is a 2 color procedural pattern player with 15 pre defined patterns. An instance is playing an overlay
   4. on the WavePlayer described above.
  
   5. 4. LightPlayer4 is a 4 color version of LightPlayer2. An instance is playing a series of patterns to the
   5. 16x16 light array on the right. It has a special "pattern 100" (they're numbered) which plays from Byte data in a
   6. user supplied array ( of uint8_t ). It displays 4 color images stored at 2 bits per light assignment.
