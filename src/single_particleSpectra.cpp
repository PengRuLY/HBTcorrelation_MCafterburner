#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "parameters.h"
#include "single_particleSpectra.h"

using namespace std;

singleParticleSpectra::singleParticleSpectra(ParameterReader *paraRdr_in, string path_in, particleSamples *particle_list_in)
{
    paraRdr = paraRdr_in;
    path = path_in;
    particle_list = particle_list_in;

    order_max = paraRdr->getVal("order_max");
    Qn_vector_real = new double [order_max];
    Qn_vector_imag = new double [order_max];
    Qn_diff_vector_real = new double* [order_max];
    Qn_diff_vector_imag = new double* [order_max];

    npT = paraRdr->getVal("npT");
    pT_min = paraRdr->getVal("pT_min");
    pT_max = paraRdr->getVal("pT_max");
    dpT = (pT_max - pT_min)/(npT - 1 + 1e-15);
    pT_array = new double [npT];
    for(int i = 0; i < npT; i++)
        pT_array[i] = pT_min + dpT*i;
    for(int i = 0; i < order_max; i++)
    {
        Qn_vector_real[i] = 0.0;
        Qn_vector_imag[i] = 0.0;
        Qn_diff_vector_real[i] = new double [npT];
        Qn_diff_vector_imag[i] = new double [npT];
        for(int j = 0; j < npT; j++)
        {
            Qn_diff_vector_real[i][j] = 0.0;
            Qn_diff_vector_imag[i][j] = 0.0;
        }
    }
    total_number_of_events = 0;

    rap_type = paraRdr->getVal("rap_type");
    rap_min = paraRdr->getVal("rap_min");
    rap_max = paraRdr->getVal("rap_max");

}

singleParticleSpectra::~singleParticleSpectra()
{
    delete [] pT_array;
    delete [] Qn_vector_real;
    delete [] Qn_vector_imag;
    for(int i = 0; i < order_max; i++)
    {
        delete [] Qn_diff_vector_real[i];
        delete [] Qn_diff_vector_imag[i];
    }
}

void singleParticleSpectra::calculate_Qn_vector_shell()
{
    int event_id = 0;
    while(!particle_list->end_of_file())
    {
        particle_list->read_in_particle_samples();
        int nev = particle_list->get_number_of_events();
        for(int iev = 0; iev < nev; iev++)
        {
            event_id++;
            cout << "Processing event: " << event_id << endl;
            int number_of_particles = particle_list->get_number_of_particles(iev);
            calculate_Qn_vector(iev);
        }
    }
    total_number_of_events = event_id;
    output_Qn_vectors();
}

void singleParticleSpectra::calculate_Qn_vector(int event_id)
{
    int number_of_particles = particle_list->get_number_of_particles(event_id);
    for(int i = 0; i < number_of_particles; i++)
    {
        double pz_local = particle_list->get_particle(event_id, i).pz;
        double E_local = particle_list->get_particle(event_id, i).E;

        double rap_local;
        if(rap_type == 0)
        {
            double mass = particle_list->get_particle(event_id, i).mass;
            double pmag = sqrt(E_local*E_local - mass*mass);
            rap_local = 0.5*log((pmag + pz_local)/(pmag - pz_local));
        }
        else
            rap_local = 0.5*log((E_local + pz_local)/(E_local - pz_local));

        if(rap_local > rap_min && rap_local < rap_max)
        {
            double px_local = particle_list->get_particle(event_id, i).px;
            double py_local = particle_list->get_particle(event_id, i).py;
            double p_perp = sqrt(px_local*px_local + py_local*py_local);
            if(p_perp > pT_min && p_perp < pT_max)
            {
                double p_phi = atan2(py_local, px_local);
                int p_idx = (int)((p_perp - pT_min)/dpT);
                for(int iorder = 0; iorder < order_max; iorder++)
                {
                    double cos_nphi = cos(iorder*p_phi);
                    double sin_nphi = sin(iorder*p_phi);
                    Qn_vector_real[iorder] += cos_nphi;
                    Qn_vector_imag[iorder] += sin_nphi;
                    Qn_diff_vector_real[iorder][p_idx] += cos_nphi;
                    Qn_diff_vector_imag[iorder][p_idx] += sin_nphi;
                }
            }
        }
    }
}

void singleParticleSpectra::output_Qn_vectors()
{
    // pT-integrated flow
    ostringstream filename;
    filename << path << "/particle_vndata.dat";
    ofstream output(filename.str().c_str());

    double dN_ev_avg = Qn_vector_real[0]/total_number_of_events;
    for(int iorder = 0; iorder < order_max; iorder++)
    {
        double Qn_evavg_real = Qn_vector_real[iorder]/total_number_of_events;
        double Qn_evavg_imag = Qn_vector_imag[iorder]/total_number_of_events;
        double vn_real = Qn_evavg_real/dN_ev_avg;
        double vn_imag = Qn_evavg_imag/dN_ev_avg;
        output << scientific << setw(18) << setprecision(8) << iorder << "   " 
               << Qn_evavg_real << "   " << Qn_evavg_imag << "   " 
               << vn_real << "   " << vn_imag << endl;
    }
    output.close();
    
    // pT-differential flow
    ostringstream filename_diff;
    filename_diff << path << "/particle_vndata_diff.dat";
    ofstream output_diff(filename_diff.str().c_str());

    for(int ipT = 0; ipT < npT; ipT++)
    {
        double dNpT_ev_avg = Qn_diff_vector_real[0][ipT]/total_number_of_events;
        output_diff << scientific << setw(18) << setprecision(8) 
                    << pT_array[ipT] << "   " << dNpT_ev_avg << "   ";
        for(int iorder = 1; iorder < order_max; iorder++)
        {
            double vn_evavg_real = Qn_diff_vector_real[iorder][ipT]/total_number_of_events/dNpT_ev_avg;
            double vn_evavg_imag = Qn_diff_vector_imag[iorder][ipT]/total_number_of_events/dNpT_ev_avg;
            output_diff << scientific << setw(18) << setprecision(8) 
                        << vn_evavg_real << "   " << vn_evavg_imag << "   ";
        }
        output_diff << endl;
    }
    output_diff.close();
}