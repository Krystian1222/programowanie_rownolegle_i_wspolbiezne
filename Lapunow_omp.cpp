#include <iostream>
#include <cmath>
#include <omp.h>
using namespace std;

class Lapunov{
     public:
        char Sequence[16];
        const int noColors = 255;
		float MinR, MaxR, MinX, MaxX;
		char NoSeq = 16;
		float LapMin=-2;
		int sample;

		int DecToBin(int Dec)
		{
			int pow;
			char rest;
			pow = 65536;
			rest = 0;

			while(rest==0 && pow>0)
			{
				rest = (char)floor(Dec/pow);
				if (rest == 0)
					pow = floor(pow/2);
			}
			while(pow>1)
			{
				Dec = Dec - pow*rest;
				pow = floor(pow/2);
				rest = (char)floor(Dec/pow);
				NoSeq++;
				Sequence[NoSeq]=rest;
			}
			return 0;
		}

		double Draw(float Seed, int NoIter, int RozX, int RozY, int start, int koniec,
		float RXMin, float RXMax, float RYMin, float RYMax,
		int val, int s, int lw, omp_sched_t typSchedule, omp_sched_t typSchedule2, int CS, char ***pixels,
		long long int **sumy)
		{
            float rx, ry, deltaX, deltaY, tmpLap=0;
            int k, w;
            char tmp;

            for(k = 0; k<16; k++)
                Sequence[k] = 0;

            sample = s;
            NoSeq = 0;
            Sequence[0] = 1;
            DecToBin(val);
            LapMin = -2;
            MinR = RXMin;
            MaxR = RXMax;
            MinX = RYMin;
            MaxX = RYMax;
            deltaX = (MaxR-MinR)/RozX;
            deltaY = (MaxX-MinX)/RozY;
            rx = MinR;
            ry = MaxX-(start-1)*deltaY;
            double z;
            int id_x, id_y;
            omp_set_nested(1);
            double start_c = omp_get_wtime();
            #pragma omp parallel private(id_y) num_threads(lw)
            {
                omp_set_schedule(typSchedule, CS);
                id_y = omp_get_thread_num();
                #pragma omp for schedule(runtime) private(w)
                    for(w=0; w<RozX; w++)
                    #pragma omp parallel private(id_x) shared(id_y) num_threads(lw)
                    {
                        int licznik = 0;
                        omp_set_schedule(typSchedule2, CS);
                        id_x = omp_get_thread_num();
                        #pragma omp for schedule(runtime) private(k)
                            for(k=0; k<RozY; k++)
                            {
                                rx = MinR + deltaX * k;
                                ry = MaxX-(start-1)*deltaY - deltaY * w;
                                tmpLap=ValLap(Seed, NoIter, rx, ry);

                                if (tmpLap<=0)
                                {
                                    z = noColors*tmpLap/LapMin;
                                    tmp = (int)(z)%noColors;
                                    pixels[k][w][0] = tmp;
                                    pixels[k][w][1] = tmp;
                                    pixels[k][w][2] = tmp;
                                }
                                else{
                                    pixels[k][w][0] = 0;
                                    pixels[k][w][1] = 0;
                                    pixels[k][w][2] = 0;
                                }
                                licznik++;


                            }
                            sumy[id_y][id_x] += (w+k);

                    }
            }
            double end = omp_get_wtime();
            double czas = end - start_c;
			return czas;
		}

		float ValLap(float Seed, int NoIter, float rx, float ry)
		{

			float x, sumlap, elem, ValLap;
			int i, poz, NoElem;
			float R;

			x = Seed;
			sumlap = 0;
			NoElem=0;
			poz=0;

			for(i=1; i<=NoIter; i++)
			{
				if (Sequence[poz]==0)
					R=ry;
				else
					R=rx;
				poz++;
				if (poz>NoSeq)
					poz=0;
				x=fun(x, R);
				elem=(float)abs(dfun(x,R));
				if (elem>1000)
				{
					ValLap=10;
					break;
				}
				if (elem!=0)
				{
					sumlap=sumlap+(float)log2(elem);
					NoElem++;
				}
			}
			if (NoElem>0)
				ValLap = sumlap/NoElem;
			else
				ValLap = 0;
			return ValLap;
		}

		float fun(float x, float r)
		{
			float y=0;
			switch(sample)
			{
				case(0) : y = r*sin(x)+r; break;
				case(1) : y = r*cos(x)+r; break;
				case(2) : y = r*cos(x)*(1-sin(x)); break;
			}
			return y;
		}

		float dfun(float x, float r)
		{
			float y=0;
			switch(sample)
			{
				case(0) : y = r*cos(x); break;
				case(1) : y = -r*sin(x); break;
				case(2) : y = r*(1-sin(x))-2*cos(x)*cos(x); break;
			}
			return y;
		}
};

int main(int argc, char **argv){
    srand(time(NULL));
    FILE * fp;
    const char *filename="Lapunow_omp.ppm";
    const char *comment="# ";
    const int MaxColorComponentValue=255;
    int rzmr = atoi(argv[1]);
    char ***pixels = new char **[rzmr]();
    for(int i = 0; i < rzmr; i++)
    {
        pixels[i] = new char*[rzmr]();
        for(int j = 0; j < rzmr; j++)
        {
            pixels[i][j] = new char[3]();
        }
    }

    int lw = atoi(argv[2]);
    string typSchedule = argv[3];
    string typScheduleWew = argv[4];
    int CS = atoi(argv[5]);
    omp_sched_t omp_typSchedule;
    omp_sched_t omp_typScheduleWew;
    if(typSchedule == "static")
        omp_typSchedule = omp_sched_static;
    else if(typSchedule == "dynamic")
        omp_typSchedule = omp_sched_dynamic;
    else if(typSchedule == "guided")
        omp_typSchedule = omp_sched_guided;

    if(typScheduleWew == "static")
        omp_typScheduleWew = omp_sched_static;
    else if(typScheduleWew == "dynamic")
        omp_typScheduleWew = omp_sched_dynamic;
    else if(typScheduleWew == "guided")
        omp_typScheduleWew = omp_sched_guided;



    long long int **sumy = new long long int*[lw]();
    for(int i = 0;i < lw; i++)
    {
        sumy[i] = new long long int[lw]();
    }
    Lapunov lp;
    fp = fopen(filename,"wb");
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,rzmr,rzmr,MaxColorComponentValue);
    double czas = lp.Draw(5, 100, rzmr,rzmr, 0, rzmr,
    -3, 9, -5, 2, 2477, 1, lw, omp_typSchedule, omp_typScheduleWew, CS, pixels, sumy);
    cout << rzmr<<";"<<lw<<";"<<typSchedule<<";"<<typScheduleWew<<";"<<CS<<";"<<czas<<";";

    for(int i = 0; i < lw; i++)
    {
        for(int j = 0; j < lw; j++)
        {
            cout << sumy[i][j] << ";";
        }
        cout << ";";
    }
    cout << endl;

    for(int i = 0; i < rzmr; i++)
    {
        for(int j = 0; j < rzmr; j++)
        {
            fwrite(pixels[i][j], 1, 3, fp);
        }
    }

    for(int i = 0; i < rzmr; i++)
    {
        for(int j = 0; j < rzmr; j++)
            delete [] pixels[i][j];
        delete [] pixels[i];
    }

    for(int i = 0; i < lw; i++)
        delete [] sumy[i];
    delete [] pixels;
    delete [] sumy;
    fclose(fp);

    return 0;
}
