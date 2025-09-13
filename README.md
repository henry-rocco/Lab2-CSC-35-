# network_labs

## Projeto Servidor e Cliente em C

### Descrição do Projeto (PT-BR)

Este projeto consiste na implementação de um **servidor** e um **cliente** em linguagem C, utilizando **soquetes TCP**.  
O servidor suporta **múltiplos usuários simultâneos** através de **threads**, de forma que cada conexão é tratada independentemente.

O cliente pode enviar dois tipos de comandos para o servidor:

- **`MyGet <caminho_do_arquivo>`**  
  O servidor envia o conteúdo do arquivo solicitado, junto com o tamanho em bytes.

- **`MyLastAccess`**  
  O servidor retorna o instante de tempo do **último acesso realizado por aquele endereço IP**.  
  Caso seja a primeira requisição de um cliente, a resposta será `Last Access = Null`.

O servidor mantém o histórico de acessos com base no **endereço IP** do cliente, permitindo que a consulta de `MyLastAccess` funcione mesmo que o cliente reconecte em outra porta.

### Como compilar e executar

```bash
# Compilar todo o código
make all

# Limpar arquivos objetos e executáveis
make clean

# Executar o servidor (porta padrão 8000)
make run_server

# Executar o cliente informando IP e, opcionalmente, a porta
./client <ip_servidor> [porta]

# Exemplos:
./client 127.0.0.1
./client 192.168.1.50 8000

# Finalizar a conexão na interface do cliente
exit
