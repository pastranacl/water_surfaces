/****************************************************************************
    
    Simulation of the hydration layer on an icosahedral virus
    Copyright (C) 2022  Cesar Lopez Pastrana
    
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
****************************************************************************/

#include "icovirus.h"
#include "allvec.h"
#include "rands.h"


int main()
{
    double E0, E;           // Energy before and after spin flip
    int **mean_lattice;     // Mean lattice configuration after equilibration
    double *energymcs;      // Energy per monte carlo step
    long *idum; 
    long foornd; 
    
    FILE *fid_lattice;
    FILE *fid_energymc;
    
    foornd = -time(NULL);
    idum = &foornd;
    
    // PRELIMINARY STEPS   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
    
    // Generate the lattice and memory allocation
    struct Lattice lattice;
    struct Virus virus;
    struct Params params;
    
    
    
    //Assign parameters
    
    virus.Rc = 50*10;
    virus.t =  3*10;
    virus.theta0 = 0.0;
    
    params.eps = EPSNN;       
    params.mu = MU;       
    params.bsurf = BSURF;
    params.beta = BETA;      
    
    
    
    gen_init(&lattice, &virus);
    
    
    // Fill the empty space randomly with water
    for(int i=0; i<lattice.nh; i++){
        for(int j=0; j<lattice.nw; j++){
            if(lattice.lattice[i][j]!=2){
                 lattice.lattice[i][j] = 1;
                /*
                if(ran1(idum)>0.5)
                    lattice.lattice[i][j] = 1;
                else
                    lattice.lattice[i][j] = -1;
                */
            }
        }
    }
    
    // Allocate memmory for the mean lattice and fill it with zeros
    mean_lattice = imatrix(lattice.nh, lattice.nw);
    for(int i=0;i<lattice.nh; i++) {
        for(int j=0; j<lattice.nw; j++)
            mean_lattice[i][j]=0;
    }
    
    
    // Save the initial lattice
    fid_lattice = fopen(FNAME_LATTICE_0, "w");
    for(int i=0; i<lattice.nh; i++){
        for(int j=0; j<lattice.nw; j++){
            fprintf(fid_lattice,"%d\t", lattice.lattice[i][j]);
        }
        fprintf(fid_lattice,"\n");
    }
    fclose(fid_lattice);
    
    
   
  
    
    // Initial energy configuration
    /*
    E=0;
    energymcs = malloc(MAX_MCS*sizeof(int *));
    for(int i=0; i<lattice.nh; i++) {
        for(int j=0; j<lattice.nw; j++) {
            if(lattice.lattice[i][j] != 2)
                E += locenergy(&lattice, &params, i, j);
        }
    }
    energymcs[0] = E;
    
    //--------------------------------------------------------------------------------/
    
    // MAIN MONTE CARLO ROUTINE   ++++++++++++++++++++++++++++++++++++++++++++++++++++/
    
    for(int mcs=1; mcs<MAX_MCS; mcs++)
    {
        for(int i=0; i<lattice.nh; i++){ 
            for(int j=0; j<lattice.nw; j++)
            {
                int x,y;
                x = (int)(lattice.nh)*ran1(idum);
                y = (int)(lattice.nw)*ran1(idum);
                if(lattice.lattice[x][y]!=2) { // Exclude the surfaces
                    
                    E0 = locenergy(&lattice, &params, x, y);
                    lattice.lattice[x][y] *= -1;
                    E = locenergy(&lattice, &params, x, y);

                    if(ran1(idum) > exp(-(E-E0)*params.beta) )
                        lattice.lattice[x][y] *= -1;
                }
            }
        }
        
        
        E=0;
        for(int i=0; i<lattice.nh; i++) {
            for(int j=0; j<lattice.nw; j++) {
                
                // Obtain average after minimisation
                if(mcs>EQ_MCS) {
                    if(lattice.lattice[i][j]==-1)
                        mean_lattice[i][j]++;
                    else if(lattice.lattice[i][j]==2)
                        mean_lattice[i][j]= -2;
                }
                
                // Calculate the total energy of the configuration
                if(lattice.lattice[i][j] != 2)
                    E += locenergy(&lattice, &params, i, j);
            }
        }
        energymcs[mcs] = E;
        
    }
    
    //--------------------------------------------------------------------------------/
    
    
    // SAVE DATA   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
    
    // Save energy evolution (normalised)
    fid_energymc = fopen(FNAME_MCSWEEPS, "w");
    for(int mcs=0; mcs<MAX_MCS; mcs++)
        fprintf(fid_energymc,"%d\t%f\n", mcs, energymcs[mcs]/EPSNN);
    fclose(fid_energymc);

    
    
    // Save one snapshot with the minimal energy configuration configuration
    fid_lattice = fopen(FNAME_LATTICE_MIN, "w");
    for(int i=0; i<lattice.nh; i++){
        for(int j=0; j<lattice.nw; j++){
            if(lattice.lattice[i][j]==1)
                fprintf(fid_lattice,"%d", 0);
            else if(lattice.lattice[i][j]== -1)
                fprintf(fid_lattice,"%d", 1);
            else
                fprintf(fid_lattice,"%d", 2);
            
            if(j<lattice.nw-1)
                fprintf(fid_lattice,"\t");
        }
        fprintf(fid_lattice,"\n");
    }
    fclose(fid_lattice);
    
    
    // Save mean configuration
    fid_lattice = fopen(FNAME_MEAN_LATTICE, "w");
    for(int i=0; i<lattice.nh; i++){
        for(int j=0; j<lattice.nw; j++){
            
            if(mean_lattice[i][j]==-2) {
                fprintf(fid_lattice,"%d", 2);
            } else {
                if( ((double)mean_lattice[i][j])/(MAX_MCS-EQ_MCS-1) > MEAN_LATTICE_THR)
                    fprintf(fid_lattice,"%d", 1);
                else
                    fprintf(fid_lattice,"%d", 0);
            }
            if(j<lattice.nw-1)
                fprintf(fid_lattice,"\t");
        }
        fprintf(fid_lattice,"\n");
    }
   fclose(fid_lattice);
   
   //--------------------------------------------------------------------------------/
   
    free(mean_lattice);
    free(lattice.lattice);
    free(energymcs);
    */
    return 0;
}


/*************************************************************/
/*                         locenergy                         */
/*************************************************************/
double locenergy(struct Lattice *lattice, 
                 struct Params *params, 
                 int y, int x)
{
    
    double E;
    int xl, xr, yt, yb;
    
    // Von Neumann neighborhood with periodic boundary conditions on the x-axis
    yt = y-1;
    yb = y+1;
    xl = x-1;
    xr = x+1;
    
    xl = xl - (int)floor((double)xl/lattice->nw)*(lattice->nw);
    xr = xr - (int)floor((double)xr/lattice->nw)*(lattice->nw);
    yt = yt - (int)floor((double)yt/lattice->nh)*(lattice->nh);
    yb = yb - (int)floor((double)yb/lattice->nh)*(lattice->nh);
    
    // Top neigthbour
    E=0;
    if(lattice->lattice[yt][x] != 2)
        E += -params->eps*lattice->lattice[yt][x]*lattice->lattice[y][x]/4.0;
    else
        E += -params->bsurf*(1.0-lattice->lattice[y][x])/2.0;
        
    // Bottom neigthbour
    if(lattice->lattice[yb][x] != 2 && yb<lattice->nw-1)
        E += -params->eps*lattice->lattice[yb][x]*lattice->lattice[y][x]/4.0;
    else
        E += -params->bsurf*(1.0-lattice->lattice[y][x])/2.0;
        
    // Left neigthbour
    if(lattice->lattice[y][xl] != 2)
        E += -params->eps*lattice->lattice[y][xl]*lattice->lattice[y][x]/4.0;    
    else
        E += -params->bsurf*(1.0-lattice->lattice[y][x])/2.0;
        
    // Rigth neightbour
    if(lattice->lattice[y][xr] != 2)
        E += -params->eps*lattice->lattice[y][xr]*lattice->lattice[y][x]/4.0;   
    else
        E += -params->bsurf*(1.0-lattice->lattice[y][x])/2.0;

    // 2.- Chemical potential
    E += (2.0*params->eps + params->mu)*(lattice->lattice[y][x])/2.0;

    return E;
}





/*************************************************************/
/*                          gen_init                         */
/* Draws the virus as icosahedron cross section              */
/*************************************************************/
void gen_init(struct Lattice *lattice, 
              struct Virus *virus)
{
    
    lattice->nw = (int)round(4.0*(virus->Rc + virus->t/2.0)/dw);
    if(lattice->nw % 2 == 0)
        (lattice->nw)++;

    lattice->nh = lattice->nw;
    lattice->lattice = imatrix(lattice->nh, lattice->nw);

    // Draw the virus
    const double dtheta = 2.0*PI/10000.0;
    const int n_theta = (int)(2.0*PI - virus->theta0)/dtheta;
    double tradius;
    double x,y;
    double theta;
    int xc,yc;
    int nls_thick;
    int center;
    
    nls_thick=(int)round(virus->t/dw);
    center = (int)floor(lattice->nw/2.0) + 1;
    
    for(int h=0; h<nls_thick; h++) { // Iterate over thickness
        tradius = h*dw + virus->Rc - virus->t/2.0;
        for(int i=0; i<=n_theta; i++) { // Iterate over angles
            
            theta = virus->theta0 + i*dtheta;
            icosahedron(tradius, theta, &x, &y);
            
            xc = (int)floor(x/dw + center);
            yc = (int)floor(y/dw + center);
            lattice->lattice[xc][yc] = 2;

        }
    }
}


/****************************************************************/
/*                          icosahedron                         */
/* Calculates the xy positions of a polar definition to draw a  */
/* a shape that resembles the cross-section of an icosahedron   */ 
/****************************************************************/
void icosahedron(double R0, double theta, double *x, double *y)
{
    const double A = 0.03;
    double pf = (1.0 + A*sin(6.0*theta));
    double r;
    
    r = sqrt(R0*R0*pf*pf);
    *x = r*cos(theta);
    *y = r*sin(theta);
}


