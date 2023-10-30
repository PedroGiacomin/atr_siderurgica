#include <iostream>
#include <Windows.h>

HANDLE event_A;
HANDLE event_ESC;

int main() {
	printf("Hello, I'm exibirAlarme.cpp\n");

	// Abre handle para o evento de encerramento
	event_ESC = OpenEvent(
		EVENT_ALL_ACCESS,			//Acesso irrestrito ao evento
		FALSE,
		(LPWSTR) "EventoESC"
	);
	if (!event_ESC)
		printf("Erro na abertura do handle para event_ESC! Codigo = %d\n", GetLastError());

	event_A = OpenEvent(
		EVENT_ALL_ACCESS,			//Acesso irrestrito ao evento
		FALSE,
		(LPWSTR) "Evento_5"
	);
	if (!event_A)
		printf("Erro na abertura do handle para event_A! Codigo = %d\n", GetLastError());

	// Trata o recebimento dos eventos
	HANDLE eventos[2] = { event_A, event_ESC };
	DWORD ret;
	int nTipoEvento = 0;

	do {	
		printf("Thread esperando evento\n");
		ret = WaitForMultipleObjects(
			2,			// Espera 2 eventos 
			eventos,	// Array de eventos que espera
			FALSE,		// Espera o que acontecer primeiro
			INFINITE	// Espera por tempo indefinido
		);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) 
			printf("Evento de bloqueio \n");
		else if (nTipoEvento == 1) 
			printf("Evento de encerramento \n");
		
		if (ret < WAIT_OBJECT_0 || ret > WAIT_OBJECT_0 + 1)
			printf("Erro no recebimento do evento! Codigo = %d\n", GetLastError());
	} while (nTipoEvento == 0); // Ate ESC ser escolhido


	std::cout << "Pressione Enter para encerrar o programa." << std::endl;
    std::cin.get();

	return EXIT_SUCCESS;
}