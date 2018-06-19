# Sisop2

[![Build Status](https://travis-ci.org/igorpf/sisop2.svg?branch=master)](https://travis-ci.org/igorpf/sisop2)

Repositório para o trabalho da cadeira de Sistemas Operacionais II - UFRGS 2018/1

## Desenvolvimento

Para trabalhar no projeto, clone o repositório:

```bash
git clone https://github.com/igorpf/sisop2.git
```

```bash
cd sisop2
```

Configurando o CMake e baixando as dependências:

```bash
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug
```

Compilando o projeto:

```bash
cmake --build _builds --config Debug
```

Nota: para desenvolver usando o CLion é indicado seguir as etapas anteriores antes de abrir o projeto.

## Uso

Para rodar o cliente:

```bash
./_builds/client/dropboxClient < nome_cliente > < ip_servidor > < porta_servidor > < porta_cliente >
```

O parâmetro porta_cliente é a porta em que o cliente escutará por novas alterações do servidor primário (frontend).

Para rodar o servidor:

```bash
./_builds/server/dropboxServer [backup | primary] < porta > [ip_server_primario] [porta_server_primario]
```
Lembrando que caso o servidor seja primário, não é necessário especificar o ip e porta do primário.
Caso seja backup, é necessário passar esses parâmetros para que o primário seja notificado.

Convenções para o uso do número das portas:
- Frontend do cliente: 8001 - 9000
- Servidor: 9001 - 10000

## Testes

Para rodar os testes:

```bash
./_builds/util/test/util_test
./_builds/client/test/client_test
./_builds/server/test/server_test
```
