#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>		// _getch
#include <string>

// Teclas a serem digitadas
#define	ESC			0x1B
#define	TECLA_1		0x31
#define TECLA_2		0x32	
#define TECLA_M		0x6D
#define TECLA_R		0x72
#define TECLA_P		0x70
#define TECLA_A		0x61
#define NUM_EVENTOS 6

// Handles para os eventos correspondentes a cada tecla
HANDLE events[6];		// Ordem dos eventos : [1, 2, m, r, p, a]
HANDLE event_ESC;		// Para a tecla esc


int main() {
	printf("Hello, I'm principal.cpp\n");

	//Caminhos relativos dos executaveis a serem inicializados
	LPCWSTR pathMensagensCLP = L"..\\Release\\mensagensCLP.exe";
	LPCWSTR pathExibirAlarme = L"..\\Release\\exibirAlarmes.exe";

	//Variaveis usadas na criacao dos processos
	STARTUPINFO siMensagensCLP, siExibirAlarme;
	PROCESS_INFORMATION piMensagensCLP, piExibirAlarme;

	BOOL status;	//retorno de erro
	
	int nTecla;		//tecla digitada

	//Criacao dos eventos de bloqueio
	for (int i = 0; i < NUM_EVENTOS; ++i) {
		std::string nomeEvento;
		nomeEvento = "Evento_" + std::to_string(i);
		std::cout << nomeEvento << std::endl;
		
		events[i] = CreateEvent(
			NULL,							// Seguranca (default)
			FALSE,							// Reset automatico
			FALSE,							// Inicia desativado
			(LPWSTR) nomeEvento.c_str()		// Nome do evento
		);
		if (!events[i])
			printf("Erro na criacao de do evento %d! Codigo = %d\n", i, GetLastError());
	}

	//Criacao do evento de encerramento
	event_ESC = CreateEvent(
		NULL,							// Seguranca (default)
		TRUE,							// Reset manual (acorda todas as threads simultaneamente)
		FALSE,							// Inicia desativado
		L"Evento_ESC"					// Nome do evento
	);
	if (!event_ESC)
		printf("Erro na criacao de do evento ESC! Codigo = %d\n", GetLastError());

	//Cria processo mensagensCLP
	ZeroMemory(&siMensagensCLP, sizeof(siMensagensCLP));
	siMensagensCLP.cb = sizeof(siMensagensCLP);
	ZeroMemory(&piMensagensCLP, sizeof(piMensagensCLP));

	status = CreateProcess(
		pathMensagensCLP,		// Nome do arquivo executavel
		NULL,					// Argumentos de linha de comando (nenhum)
		NULL,					// Atributos de seguranca do processo (default)
		NULL,					// Atributos de seguranca das threads (default)
		FALSE,					// Heranca de handles do processo criador (nao herda)
		CREATE_NEW_CONSOLE,		// Flag para inicializar com um console diferente do processo-pai
		NULL,					// Ambiente de criacao do processo (mesmo do pai)
		NULL,					// Diretorio corrente (mesmo do pai)
		&siMensagensCLP,		// Variavel que guarda informacoes de inicializacao
		&piMensagensCLP			// Struct que guarda o PID do processo entre outras infos
	);
	if (!status)
		printf("Erro na criacao de mensagensCLP! Codigo = %d\n", GetLastError());

	//Cria processo ExibirAlarme

	ZeroMemory(&siExibirAlarme, sizeof(siExibirAlarme));
	siExibirAlarme.cb = sizeof(siExibirAlarme);
	ZeroMemory(&piExibirAlarme, sizeof(piExibirAlarme));

	status = CreateProcess(
		pathExibirAlarme,		// Nome do arquivo executavel
    	NULL,					// Argumentos de linha de comando (nenhum)
		NULL,					// Atributos de seguranca do processo (default)
		NULL,					// Atributos de seguranca das threads (default)
		FALSE,					// Heranca de handles do processo criador (nao herda)
		CREATE_NEW_CONSOLE,		// Flag para inicializar com um console diferente do processo-pai
		NULL,					// Ambiente de criacao do processo (mesmo do pai)
		NULL,					// Diretorio corrente (mesmo do pai)
		&siExibirAlarme,		// Variavel que guarda informacoes de inicializacao
		&piExibirAlarme			// Struct que guarda o PID do processo entre outras infos
	);
	if (!status)
		printf("Erro na criacao de exibirAlarme! Codigo = %d\n", GetLastError());

	// Espera uma tecla ser digitada ou uma lista ficar cheia
	do {
		printf("Digite uma tecla para encerrar a tarefa que quiser\n");	//pode tirar isso depois
		nTecla = _getch();	// Isso aqui vai dar errado pra quando a lista estiver cheia
		if (nTecla == TECLA_1) 
			status = SetEvent(events[0]);
		else if (nTecla == TECLA_2)
			status = SetEvent(events[1]);
		else if (nTecla == TECLA_M)
			status = SetEvent(events[2]);
		else if (nTecla == TECLA_P)
			status = SetEvent(events[3]);
		else if (nTecla == TECLA_R)
			status = SetEvent(events[4]);
		else if (nTecla == TECLA_A)
			status = SetEvent(events[5]);
		else if (nTecla == ESC)
			status = PulseEvent(event_ESC);
        
		if (!status)
			printf("Erro no disparo de algum do evento! Codigo = %d\n", GetLastError());

	} while (nTecla != ESC);

    // Esperar todos os processos e threads encerrarem

    // Fecha handle de eventos de bloqueio
    for(int i = 0; i < NUM_EVENTOS; ++i){
        status = CloseHandle(events[i]);
		if (!status)
			printf("Erro no fechamento do handle do evento %d! Codigo = %d\n", i, GetLastError());
    }

    // Fecha handle do evento ESC
    status = CloseHandle(event_ESC);
	if (!status)
		printf("Erro no fechamento do handle do evento ESC! Codigo = %d\n", GetLastError());

	std::cout << "Pressione Enter para encerrar o programa." << std::endl;
	std::cin.get();

	return EXIT_SUCCESS;
}