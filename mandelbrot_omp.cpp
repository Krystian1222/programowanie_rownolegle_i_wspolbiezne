#include <iostream>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include <pthread.h>
#include <math.h>
#include <omp.h>
#include <fstream>
using namespace std;
const int rozmiar = 800;
const int iXmax = rozmiar, iYmax = rozmiar, MaxColorComponentValue=255, IterationMax=200;
const int lw = 8;
const double CxMin=-2.5, CxMax=1.5, CyMin=-2.0, CyMax=2.0, EscapeRadius=2;
double PixelWidth=(CxMax-CxMin)/iXmax, PixelHeight=(CyMax-CyMin)/iYmax, ER2=EscapeRadius*EscapeRadius;
const char *filename = "L06_bez_schedule.ppm", *comment = "# ";
unsigned char color[iYmax][iXmax][3];
static int nr = 0;
int liczbaElementow = iYmax * iXmax, leNaWatek = liczbaElementow / lw, licznik = 0, Iteration;
int r_bloku = 100;

int main()
{
    FILE *fp = fopen(filename, "wb");
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,iXmax,iYmax,MaxColorComponentValue);

    int i;
    int omp_get_num_threads(void);
    double start;
    int id;
    int r[8] = {255, 0, 0, 45, 0, 0, 45, 255};
    int g[8] = {0, 255, 0, 0, 45, 0, 0, 255};
    int b[8] = {0, 0, 255, 0, 0, 45, 0, 255};
    int iX, iY;
    double Cx, Cy;
    double PixelWidth =(CxMax-CxMin)/iXmax;
    double PixelHeight = (CyMax-CyMin)/iYmax;
    double Zx, Zy;
    double Zx2, Zy2;
    double ER2 = EscapeRadius*EscapeRadius;
    int Iteration;
    start = omp_get_wtime();
    int suma[lw];
    for(int i = 0; i < lw; i++)
    	suma[i] = 0;

    #pragma omp parallel private (Cx, Cy, Zx, Zy, Zx2, Zy2, Iteration, iX, iY, id), num_threads(lw)
    {
        id = omp_get_thread_num();
        #pragma omp for
        for(iY = 0; iY < iYmax; iY++)
        {
            Cy=CyMin +iY*PixelHeight;
            if(fabs(Cy) < PixelHeight/2) Cy = 0.0;

            for(iX = 0; iX < iXmax; iX++)
            {
                Cx = CxMin + iX*PixelWidth;
                Zx = 0.0;
                Zy = 0.0;
                Zx2 = Zx * Zx;
                Zy2 = Zy*Zy;
                for(Iteration=0;Iteration<IterationMax && ((Zx2+Zy2) < ER2); Iteration++)
                {
                    Zy=2*Zx*Zy + Cy;
                    Zx = Zx2-Zy2 +Cx;
                    Zx2 = Zx*Zx;
                    Zy2 = Zy *Zy;
                }
                suma[id] += Iteration;
                if(Iteration==IterationMax)
                {
                    color[iY][iX][0] = 0;
                    color[iY][iX][1] = 0;
                    color[iY][iX][2] = 0;
                }
                else
                {
                	//cout << id << endl;
                    color[iY][iX][0] = r[id];
                    color[iY][iX][1] = g[id];
                    color[iY][iX][2] = b[id];
                }
            }
        }

    }
    double end = omp_get_wtime();
    double czas = end - start;
    cout << "Rozmiar obrazu: " << iXmax << ", liczba wątków: " << lw << ", czas wykonania: " << czas << " sekund. " << endl;
    cout << "Suma iteracji pętli wewnątrz wątków:" << endl;

    //ofstream stat("L06_2.csv", ios::app);
    //stat << "Bez schedule,Rozmiar obrazu,Liczba wątków,Czas wykonania [s],Suma iteracji pętli wewnątrz wątków" << endl;
    //stat << "Guided," << rozmiar << "," << lw << ","<< r_bloku  << ","<< czas << ",";
   // for(int i = 0; i < lw; i++){
   // 	cout << suma[i] << endl;
  //  	stat << suma[i] << ",";
    //}
    //stat << endl;
    fwrite(color,1,3*iXmax*iYmax,fp);
    fclose(fp);
    //stat.close();
    return 0;
}
