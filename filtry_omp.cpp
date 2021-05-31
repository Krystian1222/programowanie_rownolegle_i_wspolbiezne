#include <iostream>
#include <omp.h>
#include <string>
#include <math.h>
#define CREATOR "KLEBKUCHEN"
#define RGB_COMPONENT_COLOR 255
using namespace std;
typedef struct{
    unsigned char color[3];
} PPMPixel;

typedef struct{
    int x, y;
    PPMPixel *data;
} PPMImage;

static PPMImage *readPPM(const char *filename){
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    fp = fopen(filename, "rb");
    if (!fp){
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    if (!fgets(buff, sizeof(buff), fp)){
        perror(filename);
        exit(1);
    }

    if (buff[0] != 'P' || buff[1] != '6'){
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img){
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    c = getc(fp);
    while (c == '#'){
        while (getc(fp) != '\n') ;
        c = getc(fp);
    }

    ungetc(c, fp);
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2){
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    if (fscanf(fp, "%d", &rgb_comp_color) != 1){
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    if (rgb_comp_color!= RGB_COMPONENT_COLOR){
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n') ;
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));
    if (!img){
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    if (fread(img->data, 3 * img->x, img->y, fp) != img->y){
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}
void writePPM(const char *filename, PPMImage *img)
{
    FILE *fp;
    fp = fopen(filename, "wb");
    if (!fp){
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    fprintf(fp, "P6\n");
    fprintf(fp, "# Created by %s\n",CREATOR);
    fprintf(fp, "%d %d\n",img->x,img->y);
    fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

int filtr_usredniajacy(PPMImage *obraz, int x, int y, int kanal)
{
    int x_nowy = x-1;
    int y_nowy = y-1;
    int suma = 0;
    int licznik = 0;
    for(int i = x_nowy; i < x_nowy + 3; i++)
        for(int j = y_nowy; j < y_nowy + 3; j++)
            if(i >= 0 && i < obraz->x)
                if(j >= 0 && j < obraz->y)
                {
                    suma+=obraz->data[i+j*obraz->x].color[kanal];
                    licznik++;
                }
    return suma/licznik;
}

int* filtr_Kuwahara(PPMImage *obraz, int lw, int CS, omp_sched_t sched)
{
    int srodek_x = obraz->x/2;
    int srodek_y = obraz->y/2;
    int start_x, stop_x, start_y, stop_y;
    int rozmiar = srodek_x * srodek_y;
    int sr[4], sg[4], sb[4];
    omp_set_nested(1);
    #pragma omp parallel private(start_x, stop_x, start_y, stop_y, srodek_x, srodek_y, sr, sg, sb, obraz, sched) num_threads(lw)
   	{
   		omp_set_schedule(sched, CS);
   		#pragma omp for schedule(runtime)
   		for(int c = 0; c < 4; c++){
    	    switch(c){
    	    case 0: start_x = 0; stop_x = srodek_x; start_y = 0; stop_y = srodek_y; break;
    	    case 1: start_x = srodek_x; stop_x = obraz->x; start_y = 0; stop_y = srodek_y; break;
    	    case 2: start_x = 0; stop_x = srodek_x; start_y = srodek_y; stop_y = obraz->y; break;
    	    case 3: start_x = srodek_x; stop_x = obraz->x; start_y = srodek_y; stop_y = obraz->y; break;
    	    }
    	    int mr=0, mg=0, mb=0;
    	    for(int x = start_x; x <= stop_x; x++){
    	        for(int y = start_y; y <= stop_y; y++){
    	            mr += obraz->data[x+y*obraz->x].color[0]/rozmiar;
    	            mg += obraz->data[x+y*obraz->x].color[1]/rozmiar;
    	            mb += obraz->data[x+y*obraz->x].color[2]/rozmiar;
    	        }
    	    }

    	    for(int x = start_x; x <= stop_x; x++){
    	        for(int y = start_y; y <= stop_y; y++){
    	            sr[c] += pow(((obraz->data[x+y*obraz->x].color[0])-mr), 2);
    	            sg[c] += pow(((obraz->data[x+y*obraz->x].color[1])-mg), 2);
    	            sb[c] += pow(((obraz->data[x+y*obraz->x].color[2])-mb), 2);
    	        }
    	    }
    	}
	}
    int minr = 0, ming = 0, minb = 0;
    for(int i = 0; i < 4; i++){
        if(sr[i] < sr[minr])
            minr = i;
        if(sg[i] < sg[ming])
            ming = i;
        if(sb[i] < sb[minb])
            minb = i;
    }
    int *tab = new int[3];
    tab[0] = sr[minr];
    tab[1] = sg[ming];
    tab[2] = sb[minb];
    return tab;
}
void changeColorPPM(PPMImage *zrodlo, PPMImage *cel, bool Kuwahara, int lw, omp_sched_t sched, int CS)
{
	omp_set_nested(1);
    if(zrodlo)
    {
        int **FK = new int*[lw];
        for(int i = 0; i < lw; i++)
            FK[lw] = new int[3];
        #pragma omp parallel private(Kuwahara, FK, lw, sched, CS) num_threads(lw)
        {
        	omp_set_schedule(sched, CS);
            if(Kuwahara)
                for(int i = 0; i < lw; i++)
                    FK[i] = filtr_Kuwahara(zrodlo, lw, CS, sched);

            #pragma omp for schedule(runtime) private(FK)
            for(int i = 0; i < zrodlo->x; i++)
                for(int j = 0; j < zrodlo->y; j++)
					for(int k = 0; k < 3; k++)
		                if(Kuwahara)
		                    for(int l = 0; l < lw; l++)
		                        cel->data[i+j*zrodlo->x].color[k]=FK[l][k];
		                else
		                	cel->data[i+j*zrodlo->x].color[k]=filtr_usredniajacy(zrodlo, i, j, k);
        }
    }
}
int main(int argc, char **argv)
{
    string nazwa = argv[1];
    int lw = atoi(argv[2]);
    string typSchedule = argv[3];
    int CS = atoi(argv[4]);
    omp_sched_t omp_typSchedule;
    if(typSchedule == "static")
        omp_typSchedule = omp_sched_static;
    else if(typSchedule == "dynamic")
        omp_typSchedule = omp_sched_dynamic;
    else if(typSchedule == "guided")
        omp_typSchedule = omp_sched_guided;

    PPMImage *obraz1;
    PPMImage *obraz2;
    PPMImage *obraz3;
    PPMImage *obraz4;
    double pocz;
    double end;
    obraz1 = readPPM(nazwa.c_str());
    obraz2 = readPPM(nazwa.c_str());
    obraz3 = readPPM(nazwa.c_str());
    obraz4 = readPPM(nazwa.c_str());
    pocz = omp_get_wtime();
    changeColorPPM(obraz1, obraz2, false, lw, omp_typSchedule, CS);
    end = omp_get_wtime();
    cout << "Uśredniający;"<<obraz1->x<<"x"<<obraz1->y<<";"<<lw<<";"<<typSchedule<<";"<<CS<<";"<<end-pocz<<endl;
    string nazwa_zapis;
    nazwa_zapis = "U_" + nazwa;
    writePPM(nazwa_zapis.c_str(), obraz2);
    nazwa_zapis = "K_" + nazwa;
    pocz = omp_get_wtime();
    changeColorPPM(obraz3, obraz4, true, lw, omp_typSchedule, CS);
	end = omp_get_wtime();
    cout << "Kuwahara;"<<obraz1->x<<"x"<<obraz1->y<<";"<<lw<<";"<<typSchedule<<";"<<CS<<";"<<end-pocz<<endl;
    writePPM(nazwa_zapis.c_str(), obraz4);
    return 0;
}
