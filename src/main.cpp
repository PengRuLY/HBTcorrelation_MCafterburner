//=============================================================================
//  calculate the HBT correlation functions from MC samples of particles
//
//  Programmer: Chun Shen
//       Email: chunshen@physics.mcgill.ca
//
//        Date: 10/31/14
//
//  To do:
//
//=============================================================================

#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
#include<math.h>
#include<sys/time.h>

#include "Stopwatch.h"
#include "parameters.h"
#include "arsenal.h"
#include "ParameterReader.h"
#include "particleSamples.h"
#include "HBT_correlation.h"
#include "single_particleSpectra.h"

using namespace std;

int main(int argc, char *argv[])
{
   // program title
   cout << endl
        << "               iHBT_afterbuner         " << endl
        << endl
        << "  Ver 1.0   ----- Chun Shen, 10/2014   " << endl;
   cout << endl << "**********************************************************" << endl;
   display_logo(2); // Hail to the king~
   cout << endl << "**********************************************************" << endl << endl;
   
   // Read-in parameters
   ParameterReader *paraRdr = new ParameterReader;
   paraRdr->readFromFile("parameters.dat");
   paraRdr->readFromArguments(argc, argv);
   paraRdr->echo();
   
   string path="results";

   Stopwatch sw;
   Stopwatch sw_total;
   sw_total.tic();
   sw.tic();

   particleSamples particle_list(paraRdr, path);
   //singleParticleSpectra testSP(paraRdr, path, &particle_list);
   //testSP.calculate_Qn_vector_shell();
   HBT_correlation test(paraRdr, path, &particle_list);
   test.calculate_HBT_correlation_function();

   sw_total.toc();
   cout << "Program totally finished in " << sw_total.takeTime() << " sec." << endl;
   return 0;
}
