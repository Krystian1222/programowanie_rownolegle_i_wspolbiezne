#include <iostream>
#include <ctime>
#include <deque>
#include <omp.h>
using namespace std;

enum kierunek
{
    polnoc = 0,
    wschod = 1,
    poludnie = 2,
    zachod = 3
};

class Punkt
{
    public:
    int x;
    int y;
    Punkt(){
        x = y = 0;
    }
    Punkt(int x, int y){
        this->x = x;
        this->y = y;
    }
};

const int rozmiar = 31;
char grid[rozmiar*rozmiar];
unsigned char labirynt[rozmiar][rozmiar][3];
int labirynt2D[rozmiar][rozmiar];
omp_lock_t blokada_omp[rozmiar][rozmiar];

void rysuj(Punkt arg){
    deque<Punkt> otoczenie;
    int x = arg.x;
    int y = arg.y;
    int id = omp_get_thread_num();
    bool flaga = false;
    do
    {
        flaga = false;
        omp_set_lock(&blokada_omp[x][y]);
        labirynt2D[x][y] = id;
        omp_unset_lock(&blokada_omp[x][y]);

        for(int i = -1; i < 2; i += 2){
            omp_set_lock(&blokada_omp[x+i][y]);
            if(labirynt2D[x+i][y] == 0){
                Punkt p(x+i, y);
                otoczenie.push_back(p);
            }
            omp_unset_lock(&blokada_omp[x+i][y]);
            omp_set_lock(&blokada_omp[x][y+i]);
            if(labirynt2D[x][y+i] == 0){
                Punkt p(x, y+i);
                otoczenie.push_back(p);
            }
            omp_unset_lock(&blokada_omp[x][y+i]);
        }

        if(otoczenie.size() >= 1){
            flaga = true;
            Punkt p = otoczenie.front();
            otoczenie.pop_front();
            x = p.x; y = p.y;

            for(Punkt i : otoczenie)
            {
                #pragma omp task shared(i) firstprivate(arg)
                rysuj(i);
            }
        }

    }while(flaga);
    #pragma omp taskwait
}

void ResetGrid(){
    for (int i=0; i < rozmiar*rozmiar; i++){
        grid[i] = '#';
    }
}

long XYToIndex( long x, long y ){
    return y * rozmiar + x;
}

int IsInBounds( int x, int y ){
    if (x < 0 || x >= rozmiar - 1)
        return false;
    if (y <0 || y >= rozmiar - 1)
        return false;
    return true;
}

void Visit( int x, int y ){
    grid[ XYToIndex(x,y) ] = ' ';
    int dirs[4];
    dirs[0] = kierunek(polnoc);
    dirs[1] = kierunek(wschod);
    dirs[2] = kierunek(poludnie);
    dirs[3] = kierunek(zachod);
    for (int i=0; i<4; ++i){
        int r = rand() & 3;
        int temp = dirs[r];
        dirs[r]  = dirs[i];
        dirs[i]  = temp;
    }
    for (int i=0; i<4; ++i){
        int dx=0, dy=0;
        switch (dirs[i]){
            case kierunek(polnoc): dy = -1; break;
            case kierunek(poludnie): dy =  1; break;
            case kierunek(wschod):  dx =  1; break;
            case kierunek(zachod):  dx = -1; break;
        }
        int x2 = x + (dx<<1);
        int y2 = y + (dy<<1);
        if (IsInBounds(x2,y2)){
            if (grid[ XYToIndex(x2,y2) ] == '#'){
                grid[ XYToIndex(x2-dx,y2-dy) ] = ' ';
                Visit(x2,y2);
            }
        }
    }
}

void PrintGrid(){
    for (int x=0; x<rozmiar; ++x){
        for (int y=0; y<rozmiar; ++y){
            if(grid[XYToIndex(x,y)] == '#'){
                labirynt2D[x][y] = -1;
            }
            else{
                labirynt2D[x][y] = 0;
            }
        }
    }

    for(int i = 0; i < rozmiar * 5; i++){
        int x = rand() % (rozmiar - 3) + 2;
        int y = rand() % (rozmiar - 3) + 2;
        if(labirynt2D[x][y] == -1)
            labirynt2D[x][y] = 0;
    }

}

int main(){
    for(int i = 0; i < rozmiar; i++){
        for(int j = 0; j < rozmiar; j++){
            omp_init_lock(&blokada_omp[i][j]);
        }
    }
    srand( time(NULL) );
    ResetGrid();
    Visit(1,1);
    PrintGrid();

    Punkt p(1,1);
    omp_set_dynamic(0);
    #pragma omp parallel shared(p)
    {
        #pragma omp single
        rysuj(p);
    }

    FILE *fptr = fopen("labirynt_omp_31.ppm", "wb");
    fprintf(fptr,"P6\n %s\n %d\n %d\n %d\n", "# ", rozmiar, rozmiar, 255);

    for(int i = 0; i < rozmiar; i++){
        for(int j = 0; j < rozmiar; j++){
            if(labirynt2D[i][j] == -1){
                for(int k = 0; k < 3; k++){
                    labirynt[i][j][k] = 0;
                }
            }
            else{
                int klr = labirynt2D[i][j];
                unsigned char r = (klr) % 255;
                klr = klr + 100;
                unsigned char g = (klr) % 255;
                klr = klr + 100;
                unsigned char b = (klr) % 255;
                labirynt[i][j][0] = r;
                labirynt[i][j][1] = g;
                labirynt[i][j][2] = b;
            }
        }
    }
    fwrite(labirynt, 1 ,3*rozmiar*rozmiar, fptr);
    fclose(fptr);
    for(int i = 0; i < rozmiar; i++){
        for(int j = 0; j < rozmiar; j++){
            omp_destroy_lock(&blokada_omp[i][j]);
        }
    }
}
