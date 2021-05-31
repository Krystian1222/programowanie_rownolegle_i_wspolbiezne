#include <iostream>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include <pthread.h>
using namespace std;
const int iXmax = 800, iYmax = 800, MaxColorComponentValue=255, IterationMax=200, lw = 8;
const double CxMin=-2.5, CxMax=1.5, CyMin=-2.0, CyMax=2.0, EscapeRadius=2;
double PixelWidth=(CxMax-CxMin)/iXmax, PixelHeight=(CyMax-CyMin)/iYmax, ER2=EscapeRadius*EscapeRadius;
const char *filename = "new1.ppm", *comment = "# ";
unsigned char color[iYmax][iXmax][3];
static int nr = 0;
int liczbaElementow = iYmax * iXmax, leNaWatek = liczbaElementow / lw, licznik = 0, Iteration;
void *koloruj(void *arg)
{
    int klr = 250 / lw;
    nr++;
    int **l = (int**)arg;
    for(int j = 0; j < leNaWatek; j++){
        int iY = l[0][j] / iXmax, iX = l[0][j] % iYmax;
        double Cy = CyMin + iY * PixelHeight, Cx = CxMin + iX * PixelWidth;
        double Zx = 0.0, Zy = 0.0, Zx2 = Zx*Zx, Zy2=Zy*Zy;
        int Iteration;
        for(Iteration=0; Iteration<IterationMax&&((Zx2+Zy2)<ER2); Iteration++){
            Zy=2*Zx*Zy+Cy; Zx=Zx2-Zy2+Cx; Zx2=Zx*Zx; Zy2=Zy*Zy;
        }
        if(Iteration==IterationMax){
            color[iY][iX][0] = 0; color[iY][iX][1] = 0; color[iY][iX][2] = 0;
        }else{
            color[iY][iX][0] = nr * klr; color[iY][iX][1] = nr + 2* klr; color[iY][iX][2] = 200 - 20 * nr;
        }
    }
    pthread_exit(NULL);
}

int main()
{
    FILE *fp = fopen(filename, "wb");
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,iXmax,iYmax,MaxColorComponentValue);
    pthread_t watki[lw];

    int **wektory = new int*[lw];
    for(int i = 0; i < lw; i++)
        wektory[i] = new int[leNaWatek];

    for(int i = 0; i < lw; i++)
        for(int j = 0; j < leNaWatek; j++)
            wektory[i][j]=licznik++;

    auto pocz = chrono::steady_clock::now();
    for(int i = 0; i < lw; i++){
        pthread_create(&watki[i], NULL, koloruj, (void*)&wektory[i]);
        pthread_join(watki[i], NULL);
    }
    auto kon = chrono::steady_clock::now();
    double czas = chrono::duration_cast<chrono::microseconds>(kon - pocz).count() / 1000000.0;
    fwrite(color,1,3*iXmax*iYmax,fp);
    fstream wyniki;
    wyniki.open("mandelbrot_czasy", ios::out | ios::app);
    if(wyniki.good()){
        wyniki << "Rozmiar obrazu: "; wyniki.width(4);
        wyniki << iYmax << ". Liczba watkow: "; wyniki.width(2);
        wyniki << lw << ". Czas wykonania: " << czas << " sekund." << endl;
        wyniki.close();
    }
    fclose(fp);

    for(int i = 0; i < lw; i++)
        delete [] wektory[i];

    delete [] wektory;
    return 0;
}
