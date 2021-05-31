#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <iostream>
using namespace std;
FILE *fptr = fopen("ulam_w_1001.ppm", "wb");
const int N = 1001;
unsigned char kolor[N][N][3];
class Dane{
    public:
    int x1, x2, y1, y2;
    Dane(){
    }
    Dane(int a, int b, int c, int d){
        x1 = a;
        x2 = b;
        y1 = c;
        y2 = d;
    }
};
#define max(x,y) ((x) > (y) ? (x) : (y))
int ulam_get_map(int x, int y, int n){
    x -= (n - 1) / 2;
    y -= n / 2;
    int mx = abs(x), my = abs(y);
    int l = 2 * max(mx, my);
    int d = y >= x ? l * 3 + x + y : l - x - y;
    return pow(l - 1, 2) + d;
}
bool isprime(int n){
    bool flaga = 0;
    for(int p = 2; p*p <= n; p++){
        if(n % p == 0){
            flaga = 1;
            break;
        }
    }
    return flaga;
}
void *rysuj(void *arg){
    Dane *wsk = (Dane*)arg;
    pthread_t numer = pthread_self();
    unsigned char r = numer % 255;
    numer = numer + 300;
    unsigned char b = numer % 255;
    numer = numer + 300;
    unsigned char g = numer % 255;
    for (int i = wsk->x1; i < wsk->x2; ++i) {
        for (int j = wsk->y1; j < wsk->y2; ++j) {
            int n = ulam_get_map(j, i, N);
            bool pierwsza = isprime(n);
            if(pierwsza){
                kolor[i][j][0] = r;
                kolor[i][j][1] = g;
                kolor[i][j][2] = b;
            }
            else{
                kolor[i][j][0] = 0;
                kolor[i][j][1] = 0;
                kolor[i][j][2] = 0;
            }
        }
    }
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    fprintf(fptr, "P6\n %s\n %d\n %d\n %d\n","# ", N, N, 255);
    const int podzialNaWiersz = 2;
    const int podzialNaWierszKwadrat = podzialNaWiersz*podzialNaWiersz;

    pthread_t watki[podzialNaWierszKwadrat];
    Dane daneTab[podzialNaWierszKwadrat];

    int rozmiarNaWatek = (N - 1) / podzialNaWiersz;
    int indeks = 0;
    for(int i = 0; i < podzialNaWiersz; i++){
        int x = 0;
        if(i == podzialNaWiersz - 1) x = 1;
        for(int j = 0; j < podzialNaWiersz; j++){
            int y = 0;
            if(j == podzialNaWiersz - 1) y = 1;
            int x1 = i * rozmiarNaWatek;
            int x2 = i * rozmiarNaWatek + rozmiarNaWatek + x;
            int y1 = j * rozmiarNaWatek;
            int y2 = j * rozmiarNaWatek + rozmiarNaWatek + y;
            Dane dane(x1, x2, y1, y2);
            daneTab[indeks] = dane;
            indeks++;
        }
    }
    for(int i = 0; i < podzialNaWierszKwadrat; i++)
        pthread_create(&watki[i], NULL, rysuj, (void*)&daneTab[i]);

    for(int i = 0; i < podzialNaWierszKwadrat; i++)
        pthread_join(watki[i], NULL);

    kolor[N/2][N/2][0] = kolor[N/2 + 1][N/2][0];
    kolor[N/2][N/2][1] = kolor[N/2 + 1][N/2][1];
    kolor[N/2][N/2][2] = kolor[N/2 + 1][N/2][2];

    fwrite(kolor, 1, 3*N*N, fptr);
    fclose(fptr);
    return 0;
}
