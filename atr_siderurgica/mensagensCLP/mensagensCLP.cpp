
#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>	// _beginthreadex() e _endthreadex() 
#include <conio.h>		// _getch
#include "CheckForError.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// Declaração das funcões executadas pelas threads
DWORD WINAPI LeituraCLP(LPVOID index);
DWORD WINAPI RetiraMensagem();
DWORD WINAPI MonitoraAlarme();
DWORD WINAPI ExibeDadosProcesso();

// Variaveis Globais 
int NSEQ = 0;
int NSEQAlarme = 0;
int pLivre1 = 0;
int pLivre2 = 0;
int pOcupado1 = 0;
int pOcupado2 = 0;
string lista1[50]; 
string lista2[50];

// Declaração funções auxiliares

int setDIAG();  // Define aleatoriamente o valor de DIAG  em um intervalo de 0 a 55

string setNSEQ(); // Define o valor de NSEQ  formatado com 5 digítos 

string setPRESS();  // Define aleatoriamente o valor de uma pressão em um intervalo de 100 a 300 com precisão de 1 casa decimal

string setTEMP(); // Define aleatoriamente a temperatura em um intervalo de 1000 a 2000 com precisão de 1 casa decimal

int setID(); //  Define aleatoriamente um ID de 0 a 99

string getTIME(); // Retorna uma string contento a hora , o minuto e o segundo atual

void produzMensagem(string &mensagem, int ID); // Produz a mensagem de leitura do CLP

void getParametrosMensagemCLP(string& mensagem, string& NSEQ, string& ID, string&DIAG, string& pressInt, string& pressInj,
	string& tempo, string& temp); // Obtem os parametros de uma mensagem 

string getDIAG(string & mensagem); // Retorna o valor do campo DIAG de uma mensagem


// Criação de tipos para indicar o estado da thread
enum estado {
	BLOQUEADO,
	DESBLOQUEADO
};

// Criação de vetores com os apontadores para os eventos
HANDLE hEventoCLP1[3];
HANDLE hEventoCLP2[3];
HANDLE hEventoMensagem[2];
HANDLE hEventoAlarme[3];

// Criação de apontador para Mutexes e Semaforos 
HANDLE hMutexCLP;
HANDLE hLista1Livre;
HANDLE hLista2Livre;
HANDLE hLista1Ocup;
HANDLE hLista2Ocup;


int main()
{
	HANDLE hThreads[5];
	DWORD dwThreadLeituraId, dwThreadRetiraMensagemId, dwThreadMonitoraAlarmeId, dwThreadExibeDados;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int i;

	for (i = 0; i < 2; ++i) {	// cria 2 threads de leitura do CLP
		hThreads[i] = (HANDLE)_beginthreadex(
			NULL,
			0,
			(CAST_FUNCTION)LeituraCLP,	// casting necessário
			(LPVOID)i,
			0,
			(CAST_LPDWORD)&dwThreadLeituraId	// casting necessário
		);

		if (hThreads[i]) printf("Thread de Leitura do CLP %d criada com Id= %0x \n", i, dwThreadLeituraId);
	}	

	hThreads[2] = (HANDLE)_beginthreadex( // Cria a thread de retirada de mensagens 
		NULL,
		0,
		(CAST_FUNCTION)RetiraMensagem,	// casting necessário
		NULL,
		0,
		(CAST_LPDWORD)&dwThreadRetiraMensagemId);	// casting necessário
	if (hThreads[2]) printf("Thread de Retirada de Mensagens %d criada com Id= %0x \n", 2, dwThreadRetiraMensagemId);

	hThreads[3] = (HANDLE)_beginthreadex( // Cria a thread de monitoração de alarme e leitura do CLP
		NULL,
		0,
		(CAST_FUNCTION)MonitoraAlarme,	// casting necessário
		NULL,
		0,
		(CAST_LPDWORD)&dwThreadMonitoraAlarmeId);	// casting necessário
	if (hThreads[3]) printf("Thread de Monitoração de Alarme %d criada com Id= %0x \n", 3, dwThreadMonitoraAlarmeId);

	hThreads[4] = (HANDLE)_beginthreadex( // Cria a thread de monitoração de alarme e leitura do CLP
		NULL,
		0,
		(CAST_FUNCTION)ExibeDadosProcesso,	// casting necessário
		NULL,
		0,
		(CAST_LPDWORD)&dwThreadExibeDados);	// casting necessário
	if (hThreads[4]) printf("Thread de Exibição de Dados do processo %d criada com Id= %0x \n", 4, dwThreadExibeDados);

	// Criação de Mutex e Semaforos
	hLista1Livre = CreateSemaphore(NULL, 50, 50, "Lista1Livre");
	hLista2Livre = CreateSemaphore(NULL, 50, 50, "Lista2Livre");
	hLista1Ocup = CreateSemaphore(NULL, 0, 50, "Lista1Ocup");
	hLista2Ocup = CreateSemaphore(NULL, 0, 50, "Lista2Ocup");
	hMutexCLP = CreateMutex(NULL, FALSE, NULL);

	// Aguarda término das threads
	dwRet = WaitForMultipleObjects(5, hThreads, TRUE, INFINITE);
	CheckForError(dwRet == WAIT_OBJECT_0);

	// Fecha todos os handles de objetos do kernel
	for (int i = 0; i < 5 ; ++i)
		CloseHandle(hThreads[i]);

	// Fecha os handles dos objetos de sincronização
	CloseHandle(hMutexCLP);
	CloseHandle(hLista1Livre);
	CloseHandle(hLista2Livre);
	CloseHandle(hLista1Ocup);
	CloseHandle(hLista2Ocup);

	return EXIT_SUCCESS;
}	// main

DWORD WINAPI LeituraCLP(LPVOID index)
{
	int i = (int)index;
	string mensagem = "";
	DWORD dwStatus, ret;
	DWORD nTipoEvento = -1;
	estado estadoLeitura = DESBLOQUEADO; // Thread começa  desbloqueada

	do {
		if (estadoLeitura == DESBLOQUEADO) {
			produzMensagem(mensagem, i);
			WaitForSingleObject(hMutexCLP, INFINITE);
			lista1[pLivre1] = mensagem; // Coloca na memória
			pLivre1 = (pLivre1 + 1) % 50;
			ReleaseSemaphore(hLista1Ocup, 1, NULL);
			ReleaseMutex(hMutexCLP);
			Sleep(500);
		}
		else { 
				//ret = WaitForMultipleObjects(8, hEventos, FALSE, INFINITE);
				//if (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + 8) nTipoEvento = ret - WAIT_OBJECT_0;
				//if (nTipoEvento == 0) break;	// Ocorreu um ESC
				//else if (nTipoEvento == i) {    // O operador quer mudar o estado da thread 
				//	estadoLeitura = DESBLOQUEADO; continue;
				//}
		} 
		Sleep(500);
	} while (1);
	return 0;
}

DWORD WINAPI RetiraMensagem() {
	do {
		WaitForSingleObject(hLista1Ocup, INFINITE);
		string DIAG = getDIAG(lista1[pOcupado1]);
		if (DIAG != "55") { // Adiciona em outra lista circular 
			WaitForSingleObject(hLista2Livre, INFINITE);
			lista2[pLivre2] = lista1[pOcupado1];
			pLivre2 = (pLivre2 + 1) % 50;
			ReleaseSemaphore(hLista2Ocup, 1, NULL);
		}
		else { // pipes ou mailslots 
		}
		pOcupado1 = (pOcupado1 + 1) % 50;
		ReleaseSemaphore(hLista1Livre, 1, NULL);
	} while (1);
	return 0;
}

DWORD WINAPI MonitoraAlarme() {
	do {
		stringstream ss; 
		string tempo = getTIME();
		int ID = setID();
		ss << setfill('0') << setw(5) << NSEQ << ";" << setfill('0') << setw(2) << ID << ";" << tempo;
		string mensagemAlarme = ss.str();

		int tempoDormindo = rand() % 4001 + 1000;
		Sleep(tempoDormindo);

	} while (1);
	return 0;
}

DWORD WINAPI ExibeDadosProcesso() {
	do {
		string NSEQ, ID, DIAG, TEMP, PRESS_INJ, PRESS_INT, TEMPO;
		WaitForSingleObject(hLista2Ocup, INFINITE);
		getParametrosMensagemCLP(lista2[pOcupado2], NSEQ, ID, DIAG, PRESS_INT, PRESS_INJ, TEMPO, TEMP);
		cout << TEMPO << " NSEQ: " << NSEQ << " PR INT: " << PRESS_INT << " PR N2: " << PRESS_INJ << " TEMP: " << TEMP << endl;
		pOcupado2 = (pOcupado2 + 1) % 50;
		ReleaseSemaphore(hLista2Livre, 1, NULL);
	} while (1);
	return 0;
}

// Implementação das funções auxiliares
void produzMensagem(string& mensagem, int ID) {
	int DIAG = setDIAG();
	WaitForSingleObject(hMutexCLP, INFINITE);
	string SEQ = setNSEQ();
	string tempo = getTIME();
	stringstream ss;
	if (DIAG == 55) {
		ss << SEQ << ";" << ID + 1 << ";" << DIAG << ";" << "00000.0" << ";" << "00000.0" << ";" << "00000.0" <<":"<< tempo;
	}
	else {
		string PRESS_INT = setPRESS();
		string PRESS_INJ = setPRESS();
		string TEMP = setTEMP();
		ss << SEQ << ";" << ID + 1 << ";" << DIAG << ";" << PRESS_INT << ";" << PRESS_INJ << ";" << TEMP << ";" << tempo;

	}
	NSEQ++;
	ReleaseMutex(hMutexCLP);
	mensagem = ss.str();
}

int setDIAG() {
	int resultado = rand() % 70;
	if (resultado >= 55) return 55;
	else return resultado;
}

string setNSEQ() {
	ostringstream ss;
	ss << std::setfill('0') << std::setw(5) << NSEQ;
	string numeroFormatado = ss.str(); // Salvar o valor formatado em uma variável
	return numeroFormatado;
}

string setPRESS() {
	int numSorteado = rand() % 2001 + 1000;
	if (numSorteado % 10 == 0) numSorteado++;
	double num = numSorteado / 10.0;
	ostringstream ss;
	ss << "0" << num;
	string numeroFormatado = ss.str();
	return numeroFormatado;

}

string setTEMP() {
	int numSorteado = rand() % 10001 + 10000;
	if (numSorteado % 10 == 0) numSorteado++;
	double num = numSorteado / 10;
	ostringstream ss;
	ss << num;
	string numeroFormatado = ss.str();
	return numeroFormatado;
}

int setID() {
	int numSorteado = rand() % 99;
	return numSorteado;
}

string getTIME() {
	SYSTEMTIME tempo;
	GetSystemTime(&tempo);
	ostringstream ss;
	ss << tempo.wHour << ":" << tempo.wMinute << ":" << tempo.wSecond;
	string temp = ss.str();
	return temp;
}

void getParametrosMensagemCLP(string& mensagem, string& NSEQ, string& ID, string& DIAG, string& pressInt, string& pressInj, string& tempo, string& temp) {
	stringstream ss(mensagem);
	getline(ss, NSEQ, ';');
	getline(ss, ID, ';');
	getline(ss, DIAG, ';');
	getline(ss, pressInt, ';');
	getline(ss, pressInj, ';');
	getline(ss, temp, ';');
	getline(ss, tempo, ';');
}

string getDIAG(string& mensagem) {
	string auxiliar, diag;
	stringstream ss(mensagem);
	getline(ss, auxiliar, ';');
	getline(ss, auxiliar, ';');
	getline(ss, diag, ';');
	return diag;
}


