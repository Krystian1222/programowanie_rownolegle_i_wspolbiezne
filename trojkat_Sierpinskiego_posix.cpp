#include <iostream>
#include <math.h>
#include <pthread.h>

using namespace std;
const int n = 10;
const int a = pow(2, n);
unsigned char kolor[a][a][3];

class Punkty
{
    public:
    int p1, p2, p3;
    Punkty(){
        p1 = p2 = p3 = 0;
    }
    Punkty(int x, int y, int z){
        p1 = x; p2 = y; p3 = z;
    }
};
int licz = 40;
void *rysuj(void *arg)
{
    licz++;
    if(licz > 254) licz = 40;
    Punkty *wskp = (Punkty*)arg;
    int p1 = wskp->p1;
    int p2 = wskp->p2;
    int p3 = wskp->p3;
    int dl_boku = p2 - p1;
    for(int i = 0; i < dl_boku; i++)
        for(int j = i; j < dl_boku; j++)
            for(int k = 0; k < 3; k++){
                kolor[i+p3][j+p1][0] = licz / 2;
                kolor[i+p3][j+p1][1] = licz;
                kolor[i+p3][j+p1][2] = 0;
            }
    Punkty tab[3];
    tab[0] = Punkty(p1, p2 - dl_boku / 2, p3 - dl_boku / 2);
    tab[1] = Punkty(p2, p2 + dl_boku / 2, p3 + dl_boku / 2);
    tab[2] = Punkty(p1, p2 - dl_boku / 2, p3 + dl_boku / 2);

    if(dl_boku > 2){
        pthread_t watki[3];
        for(int i = 0; i < 3; i++){
            pthread_create(&watki[i], NULL, rysuj, &tab[i]);
        }
	for(int i = 0; i < 3; i++{
	    pthread_join(watki[i], NULL);
	}
    }
}

int main()
{
    for(int i = 0; i < a; i++)
        for(int j = 0; j < a; j++)
            for(int k = 0; k < 3; k++)
                kolor[i][j][k] = 0;

    FILE *fp = fopen("trojkat_kolorowy.ppm", "wb");
    const char *comment = "# ";
    fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, a, a, 255);

    Punkty p(0, a / 2, a / 2);
    pthread_t watek;
    pthread_create(&watek, NULL, rysuj, &p);
    pthread_join(watek, NULL);

    for(int i = 0; i < a; i++)
        for(int j = 0; j < a; j++)
            fwrite(kolor[i][j], 1, 3, fp);

    fclose(fp);
    return 0;
}
