// PRIW_L01_2020_10_09
// Krystian Lebkuchen
// 44397
// Grupa B
// lk44397@zut.edu.pl

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <stdlib.h>
#include <chrono>
using namespace std;

class Dane
{
	public:
	int **A;
	int **B;
	int **C;
	int N;
	int liczba_watkow;
	static int krok_st;
	Dane(int a)
	{
		N = a;
		A = new int*[N];
		B = new int*[N];
		C = new int*[N];
	}
	
	void stworz_macierz()
	{
		for(int i = 0; i < N; i++)
		{
			A[i] = new int[N];
			B[i] = new int[N];
			C[i] = new int[N];
		}	
	}
	
	void inicjalizuj_macierz()
	{
		for(int i = 0; i < N; i++)
			for(int j = 0; j < N; j++)
			{
				A[i][j] = rand() % 100 + 1;
				B[i][j] = rand() % 100 + 1;
				C[i][j] = 0;
			}
	}
	
	void wypisz_macierz_A()
	{
		for(int i = 0; i < N; i++)
		{
			for(int j = 0; j < N; j++)
			{
				cout << A[i][j] << " ";
			}
			cout << endl;
		}
	}	
	
	void wypisz_macierz_B()
	{
		for(int i = 0; i < N; i++)
		{
			for(int j = 0; j < N; j++)
			{
				cout << B[i][j] << " ";
			}
			cout << endl;
		}
	}

	void wypisz_macierz_C()
	{
		for(int i = 0; i < N; i++)
		{
			for(int j = 0; j < N; j++)
			{
				cout << C[i][j] << " ";
			}
			cout << endl;
		}
	}
	
	void usun_macierz()
	{
		for(int i = 0; i < N; i++)
		{
			delete [] A[i];
			delete [] B[i];
			delete [] C[i];
		}
	
		delete [] A;
		delete [] B;
		delete [] C;
	}
};

int Dane::krok_st = 0;

void *mnoz(void *arg)
{
	Dane *wsk_na_dane = (Dane*) arg;
	int mnoznik = wsk_na_dane->krok_st++;
	int iter = mnoznik * wsk_na_dane->N;
	int lw = wsk_na_dane->liczba_watkow;
	for(int i = (iter) / (lw); i < ((mnoznik + 1) * wsk_na_dane->N) / (lw); i++)
		for(int j = 0; j < wsk_na_dane->N; j++)
			for(int k = 0; k < wsk_na_dane->N; k++)
				wsk_na_dane->C[i][j] += wsk_na_dane->A[i][k] * wsk_na_dane->B[k][j];
}

int main()
{
	ofstream czasy("macierze_czasy");
	for(int i = 400; i < 1700; i *= 2)
	{
		Dane dane(i);
		
		dane.stworz_macierz();
		dane.inicjalizuj_macierz();
		
		// mnozenie macierzy
		auto start = chrono::steady_clock::now();
		for(int i = 0; i < dane.N; i++)
			for(int j = 0; j < dane.N; j++)
				for(int k = 0; k < dane.N; k++)
					(dane.C[i][j]) += (dane.A[i][k]) * ((dane.B[k][j]));
		auto end = chrono::steady_clock::now();
		//dane.wypisz_macierz_A();
		//dane.wypisz_macierz_B();
		//dane.wypisz_macierz_C();
	
		cout << "Rozmiar macierzy: " ;
		cout.width(4);
		cout << dane.N << ", czas sekwencyjny : " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0 << " sekund." << endl;
		czasy << "Rozmiar macierzy: ";
		czasy.width(4);
		czasy << dane.N << ". Czas sekwencyjny : " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0 << " sekund." << endl;
		
		for(int j = 1; j < 9; j *= 2)
		{
			//dane.stworz_macierz();
			dane.inicjalizuj_macierz();
			dane.liczba_watkow = j;
			pthread_t watki[j];
			dane.krok_st = 0;
			auto start = chrono::steady_clock::now();
			for(int k = 0; k < j; k++)
			{
				pthread_create(&watki[k], NULL, mnoz, &dane);
			}
		
			for(int k = 0; k < dane.liczba_watkow; k++)
				pthread_join(watki[k], NULL);
			auto end = chrono::steady_clock::now();
			
			cout << "Rozmiar macierzy: ";
			cout.width(4);
			cout << i << ", czas z ";
			cout.width(2);
			cout << j << " watkami: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0 << " sekund.\n";
			czasy << "Rozmiar macierzy: ";
			czasy.width(4);
			czasy << i << ", czas z ";
			czasy.width(2);
			czasy << j << " watkami: " << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0 << " sekund.\n";
			//dane.usun_macierz();
			//dane.wypisz_macierz_A();
			//dane.wypisz_macierz_B();
			//dane.wypisz_macierz_C();
		}
		dane.usun_macierz();
	}
	
	return 0;
}

