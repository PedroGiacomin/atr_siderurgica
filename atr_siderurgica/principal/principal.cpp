#include <iostream>
#include <Windows.h>

int main() {
	printf("Hello, I'm principal.cpp\n");

	STARTUPINFO siMensagensCLP;
	PROCESS_INFORMATION piMensagensCLP;

	BOOL status;

	ZeroMemory(&siMensagensCLP, sizeof(siMensagensCLP));
	siMensagensCLP.cb = sizeof(siMensagensCLP);
	ZeroMemory(&piMensagensCLP, sizeof(piMensagensCLP));

	//Cria processo mensagensCLP
	status = CreateProcess(
		"..\\Debug\\mensagensCLP.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&siMensagensCLP,
		&piMensagensCLP
	);
	if (!status)
		printf("Erro na criacao de mensagensCLP! Codigo = %d\n", GetLastError());

	STARTUPINFO siExibirAlarme;
	PROCESS_INFORMATION piExibirAlarme;

	ZeroMemory(&siExibirAlarme, sizeof(siExibirAlarme));
	siExibirAlarme.cb = sizeof(siExibirAlarme);
	ZeroMemory(&piExibirAlarme, sizeof(piExibirAlarme));

	//Cria processo ExibirAlarme
	status = CreateProcess(
		"..\\Debug\\exibirAlarmes.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&siExibirAlarme,
		&piExibirAlarme
	);
	if (!status)
		printf("Erro na criacao de exibirAlarme! Codigo = %d\n", GetLastError());

	std::cout << "Pressione Enter para encerrar o programa." << std::endl;
	std::cin.get();

	return EXIT_SUCCESS;
}