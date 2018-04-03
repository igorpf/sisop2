# Sisop2

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
./_builds/client/dropboxClient
```

Para rodar o servidor:

```bash
./_builds/server/dropboxServer
```

## Testes

Para rodar os testes:

```bash
./_builds/util/test/util_test
./_builds/client/test/client_test
./_builds/server/test/server_test
```
