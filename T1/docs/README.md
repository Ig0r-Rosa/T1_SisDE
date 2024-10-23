# Projeto: Medidor de Altura com Sensor Ultrassônico

Este projeto implementa um sistema para medir alturas usando um sensor ultrassônico (HC-SR04) conectado a um microcontrolador ESP32. O sistema permite calibrar a altura do chão e medir alturas dinâmicas em relação ao ponto calibrado.

## Funcionalidade

O sistema possui três principais funcionalidades acessíveis por meio de um menu interativo:

1. **Calibrar Altura**: Captura a altura atual como a altura do "chão" para referência.
2. **Mostrar Altura**: Exibe a altura em relação ao ponto calibrado continuamente.
3. **Voltar ao Menu**: Permite retornar ao menu inicial.

O sistema utiliza multitarefas (FreeRTOS) para realizar a medição de distância de forma assíncrona e para exibir a altura em tempo real.

## Estrutura do Código

- **MenuTask**: Responsável por mostrar o menu e processar as entradas do usuário.
- **CalcularDistancia**: Calcula a distância medida pelo sensor ultrassônico e armazena em uma variável compartilhada.
- **exibirAltura**: Mostra a altura em relação ao ponto calibrado até que o usuário decida parar.

Para garantir a consistência dos dados, é utilizado um mutex para proteger as leituras e escritas da distância.

## Configuração do Ambiente

### Dependências

- **ESP-IDF**: Framework de desenvolvimento para ESP32.
- **FreeRTOS**: Biblioteca para multitarefas.
- **HC-SR04**: Sensor ultrassônico para medição de distância.

### Configuração para Linux

1. **Instalar o ESP-IDF**

   Siga as instruções oficiais de instalação do ESP-IDF: [ESP-IDF para Linux](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-setup.html).
2. **Clonar o Projeto**

   ```bash
   git clone https://github.com/seu-usuario/seu-repositorio.git
   cd seu-repositorio
   ```
3. **Configurar o Projeto**

   ```bash
   idf.py set-target esp32
   idf.py menuconfig
   ```

   Certifique-se de configurar os pinos GPIO conforme definido no código (TRIG_PIN e ECHO_PIN).
4. **Compilar e Flashar**

   ```bash
   idf.py build
   idf.py flash
   ```
5. **Monitorar o Log**

   ```bash
   idf.py monitor
   ```

### Configuração para Windows

1. **Instalar o ESP-IDF**

   Siga as instruções oficiais de instalação do ESP-IDF: [ESP-IDF para Windows](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html).
2. **Clonar o Projeto**
   Abra o terminal do ESP-IDF e execute:

   ```cmd
   git clone https://github.com/seu-usuario/seu-repositorio.git
   cd seu-repositorio
   ```
3. **Configurar o Projeto**

   ```cmd
   idf.py set-target esp32
   idf.py menuconfig
   ```

   Certifique-se de configurar os pinos GPIO conforme definido no código (TRIG_PIN e ECHO_PIN).
4. **Compilar e Flashar**

   ```cmd
   idf.py build
   idf.py flash
   ```
5. **Monitorar o Log**

   ```cmd
   idf.py monitor
   ```

## Uso do Sistema

Após a configuração e flash do firmware, o sistema iniciará e exibirá o menu no monitor serial. Utilize as seguintes opções para interagir com o sistema:

- **[1]** Calibrar a altura do chão.
- **[2]** Mostrar a altura atual em relação ao ponto calibrado.
- **[0]** Retornar ao menu principal.

## Considerações

- **Segurança**: Certifique-se de ligar corretamente os pinos do sensor para evitar danos ao microcontrolador.
- **Limitações do Sensor**: O sensor HC-SR04 possui um alcance limitado de 0,05m a 4m. Valores fora desse intervalo serão tratados como inválidos.

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir uma issue ou um pull request.

## Licença

Este projeto está sob a licença MIT - consulte o arquivo LICENSE para mais detalhes.
