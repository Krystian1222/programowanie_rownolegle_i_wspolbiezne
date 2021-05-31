#include <iostream>
#include <cmath>
#include <omp.h>
#include <fstream>

#define max(x, y) ((x) > (y) ? (x) : (y))

using namespace std;

const int NT = 8;
const int N = 801;
unsigned char kolor[N][N][3];
int r_bloku = 150;
int ulam_get_map(int x, int y, int n)
{
    x -= (n - 1) / 2;
    y -= n / 2;

    int mx = abs(x), my = abs(y);
    int l = 2 * max(mx, my);
    int d = y >= x ? l * 3 + x + y : l - x - y;

    return pow(l - 1, 2) + d;
}

bool isprime(int num){
    int p;
    for(p = 2; p*p <= num; p++){
        if(num % p == 0){
            return 0;
        }
    }
    return num >= 2;
}

int main()
{
    int i, j;
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
            for(int k = 0; k < 3; k++)
                kolor[i][j][k] = 0;
    int id_x, id_y;
    omp_set_nested(1);
    long long int sumy[NT][NT] = {0};
    srand(time(NULL));
    double start = omp_get_wtime();
    #pragma omp parallel private(id_x) num_threads(NT)
    {
        id_x = omp_get_thread_num();
        #pragma omp for schedule(guided, r_bloku) private(i)
            for(i = 0; i < N; i++)
            #pragma omp parallel private(id_y) shared(id_x) num_threads(NT)
            {
                id_y = omp_get_thread_num();
                #pragma omp for schedule(guided, r_bloku) private(j)
                for(j = 0; j < N; j++)
                {
                    int v = ulam_get_map(j, i, N), p;
                    int is_prime = 1;
                    for(p = 2; p*p <= v; p++)
                        if(is_prime = v % p == 0)
                            break;
                    sumy[id_x][id_y] += p;

                    if(is_prime == 0)
                    {
                        kolor[i][j][0] = 0;
                        kolor[i][j][1] = 0;
                        kolor[i][j][2] = 0;
                    }
                    else
                    {
                        kolor[i][j][0] = ((id_x*20 + id_y*10 + 1) *20)%255;
                        kolor[i][j][1] = ((id_x*30 + id_y*10 + 1) *20)%255;
                        kolor[i][j][2] = ((id_x*40 + id_y*10 + 1) *20)%255;
                    }
                }
            }
    }
    double end = omp_get_wtime();
    double czas = end - start;
    cout << "Suma iteracji pętli wewnątrz wątków:" << endl;
    for(int i = 0; i < NT; i++)
    {
        for(int j = 0; j < NT; j++)
        {
            cout << sumy[i][j] << "\t";
        }
        cout << endl;
    }

    cout << "Rozmiar obrazu: " << N << ", liczba wątków: " << NT << ", czas wykonania: " << czas << " sekund. " << endl;

    FILE *fp = fopen("Ulam_guided.ppm", "wb");
    fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", "# ", N, N, 255);



    //ofstream stat("L07_6.csv", ios::app);
    //stat << "Bez schedule,Rozmiar obrazu,Liczba wątków,Rozmiar bloku,Czas wykonania [s],Suma iteracji pętli wewnątrz wątków" << endl;
    //stat << "Guided," << N << "," << NT << ","<< r_bloku  << ","<< czas << ",";

    for(int i = 0; i < NT; i++)
    {
        for(int j = 0; j < NT; j++)
        {
           // stat << sumy[i][j] << ",";
        }
     //   stat << endl;
       // stat << ",,,,,";
    }
    //fwrite(color,1,3*iXmax*iYmax,fp);
    //fclose(fp);
    //stat.close();


    kolor[N/2][N/2 + 1][0] = kolor[N/2 - 1][N/2 + 1][0] = 0;
    kolor[N/2][N/2 + 1][1] = kolor[N/2 - 1][N/2 + 1][1] = 0;
    kolor[N/2][N/2 + 1][2] = kolor[N/2 - 1][N/2 + 1][2] = 0;

    fwrite(kolor, 1 ,3*N*N, fp);

    fclose(fp);
}
