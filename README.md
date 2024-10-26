# Simulação: Concorrência

Desenvolvido por:
André Born `a2649136`
Otavio Luiz Gonzaga `a2648920`

<hr />

Programa em C++ com uso de threads, mutexes (lock) e variáveis de condição (semáforos) para controlar a execução concorrente de diferentes partes de um programa que simula um sistema de produção, interceptação e consumo de itens.

## Visão Geral do Programa

O programa implementa um sistema em que três threads colaboram para processar uma fila de números:

- Producer (Produtor): Gera e coloca itens na fila.
- Interceptor (Interceptor): Modifica o último item da fila antes do consumo.
- Consumer (Consumidor): Remove e consome itens da fila.

## Componentes e Estrutura

### Declarações Globais

#### Fila de inteiros compartilhada

```cpp
std::queue<int> fila;
```

Cria uma fila de inteiros que será produzida, interceptada e modificada pelo programa

#### Princípio de exculsão mútua

```cpp
std::mutex exclusao_mutua;
```

Protege o acesso simultâneo à fila, garantindo exclusão mútua.

#### Semáforos

```cpp
std::condition_variable cv;
```

Usada como um sinalizador, permite que uma thread aguarde até que outra sinalize uma mudança de condição.

```cpp
bool finished = false;
```

Indica o fim do processo de produção.

```cpp
bool intercepted = true;
```

Sinaliza se o item da fila foi modificado pelo interceptador.

### Funções

#### Função producer()

Essa função executa o papel de produção e inclui:

Um loop que gera 10 itens (de 1 a 10) e os insere na fila, pausando por 1 segundo entre cada inserção.

O bloqueio std::lock_guard\<std::mutex\> lock(exclusao_mutua); protege a fila durante as operações de inserção.

Após inserir cada item, cv.notify_one(); sinaliza uma das threads à espera (interceptor ou consumidor) de que há um novo item na fila.

Após gerar todos os itens, a função define finished como true e notifica todas as threads com cv.notify_all();, indicando que a produção terminou.

#### Função interceptor()

A função interceptor() monitora a fila para modificar itens gerados antes do consumo:

Um loop espera (cv.wait) até que um item não interceptado esteja na fila ou a produção tenha sido concluída.

O interceptor multiplica o valor do item por um número aleatório entre 0 e 9 para simular uma modificação.

Depois, marca o item como interceptado (intercepted = true) e notifica o consumidor com cv.notify_one();.

O loop encerra quando finished é true e todos os itens na fila foram interceptados.

#### Função consumer()

O consumidor remove e processa itens interceptados da fila:

Espera até que um item interceptado esteja disponível ou a produção tenha sido concluída.

Remove o item da fila e imprime o valor consumido.

O loop é encerrado quando finished é true e a fila está vazia.

## Requisitos

Durante o desenvolvimento e teste foi usada o compilador g++

```bash
gcc version 13.2.0 (Ubuntu 13.2.0-23ubuntu4)
```
