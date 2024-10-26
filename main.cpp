#include <iostream>			  // input/output streams
#include <thread>			  // permite trabalhar com threads em c++
#include <mutex>			  // servirá para aplicar o conceito de exclusão mútua
#include <condition_variable> // usado para criar uma variável global que servirá como semáforo para as threads
#include <queue>			  // usado para criar uma fila de processos
#include <chrono>			  // (opcional) usado para trabalhar com tempo

std::queue<int> fila;		// inicia uma fila de elementos do tipo int com o nome "fila"
std::mutex exclusao_mutua;	// inicia um objeto mutex
std::condition_variable cv; // inicia um "semáforo"
bool finished = false;		// indcará quando um determinado processo teminou, prevenindo deadlock
bool intercepted = true;	// indicará se o item na fila foi interceptado (inicia em true pois não há items na fila)

void producer();
void interceptor();
void consumer();

int main()
{
	// cria uma thread prod para executar a função producer
	std::thread prod(producer);
	// cria uma thread intercept para executar a função interceptor
	std::thread intercept(interceptor);
	// cria uma thread cons para executar a função consumer
	std::thread cons(consumer);

	// "join" junta as threads no processo principal
	prod.join();
	intercept.join();
	cons.join();

	return 0;
}

void producer()
{
	using namespace std::chrono_literals; // (opcional) apenas para usar as unidades de tempo da lib chrono

	// produzirá na fila 10 vezes antes de encerrar
	for (int i = 1; i <= 10; ++i)
	{
		// pausa a  thread por 1 segundo (opcional)
		std::this_thread::sleep_for(1s);

		// faz o lock para acessar a fila
		std::lock_guard<std::mutex> lock(exclusao_mutua);

		// adiciona i na fila
		fila.push(i);
		intercepted = false;

		// loga o valor produzido no console
		std::cout << "Produzido: " << i << std::endl;

		// notifica as threads que um novo item foi produzido
		cv.notify_one();
	}

	// está entre chaves para definir um escopo que o lock_guard irá proteger
	{
		// faz o lock das threads para acessar finished de maneira segura
		std::lock_guard<std::mutex> lock(exclusao_mutua);
		finished = true;
	}

	// notifica as threads que o processo de produção terminou, para que elas possam sair do loop
	cv.notify_all();
}

void interceptor()
{
	while (true)
	{

		std::unique_lock<std::mutex> lock(exclusao_mutua);

		// espera até que o produtor produza o número e diga que ele não está interceptado ou quando o processo de produção indicar o término da execução
		cv.wait(lock, []
				{ return !intercepted || finished; });

		// acessa o primeiro item da fila
		int value = fila.front();

		// modifica o item
		value *= rand() % 10;

		// remove o item não interceptado da fila
		fila.pop();

		// adiciona o item modificado na fila
		fila.push(value);

		intercepted = true;

		std::cout
			<< "Modificado: " << value << std::endl;

		cv.notify_one();

		// quebra o laço quando o produtor terminar a produção e o interceptor teminar de interceptar
		if (finished && intercepted)
			break;
	}
}

void consumer()
{
	// entra em loop infinito
	while (true)
	{
		// bloqueia o acesso até que fila esteja vazia (ver break)
		std::unique_lock<std::mutex> lock(exclusao_mutua);

		// espera por um novo item na fila ou quando o processo de produção indicar o término da execução
		// recebe uma callback que deve retornar um boolean, só continua o processo quando a condição for true
		cv.wait(lock, []
				{ return (!fila.empty() && intercepted) || finished; });

		// se a fila não estiver vazia, remove um item da fila e imprime o valor
		while (!fila.empty() && intercepted)
		{
			// acessa o primeiro item da fila
			int value = fila.front();

			// remove o item da fila
			fila.pop();

			// imprime o item consumido
			std::cout << "Consumido: " << value << "\n"
					  << std::endl;
		}

		// quebra o laço de repetição quando o produtor indicar o término da execução com finished; ou
		// quando a fila estiver vazia
		if (finished && fila.empty())
			break;
	}
}