# Checkpoint 04 - O Caso da Vinheria Agnello - IoT

O seu projeto evoluiu! Com o novo checkpoint, o foco passa a ser a integração do sistema de monitoramento da vinheria com o ecossistema de negócios da União Europeia, utilizando a plataforma **FIWARE** como backend.

📺 **Veja o vídeo explicativo do projeto 👉 [**Clique aqui**](https://youtu.be/_eXlb5e-gMw)**

---

## 👥 Equipe do Projeto

| Nome | RM |
|---|---|
| Luara Martins de Oliveira Ramos | 565573 | rm565573@fiap.com.br |
| Ana Luiza Tibiriçá da Paixão | 562098 |
| Áurea Sardinha Carminato | 563837 | 
| Laura Tigre Amaral | 565281 | 

---

## 💻 Desenvolvimento

### 🛠️ Componentes do Backend e IoT
O projeto agora integra os sensores da vinheria com uma plataforma robusta de backend.

- **Servidor FIWARE:** Backend principal da solução para interoperar com o mercado europeu.
- **Protocolo MQTT:** Utilizado para que o microcontrolador envie os dados dos sensores para o FIWARE.
- **Containers Docker:** Essenciais para rodar os serviços do FIWARE (Orion, STH-Comet, IoT Agent) em um ambiente Linux.
- **Microcontrolador (MCU):** Envia as informações dos sensores para a internet através de comunicação Wi-Fi.

---

## ⚙️ Explicação dos Componentes FIWARE

### Orion Context Broker
Gerencia os dados coletados pelos sensores em tempo real. Ele armazena as informações e as disponibiliza para outras aplicações.

### STH-Comet
Responsável por criar um histórico de todos os dados coletados. Isso é fundamental para a criação de gráficos e relatórios, atendendo aos requisitos da vinheria.

### IoT Agent MQTT
Atua como a ponte entre os sensores e a plataforma. Ele recebe os dados via MQTT e os traduz para o formato que o Orion Context Broker entende, utilizando o padrão NGSIv2.

---

## 🔁 Exibição de Informações

- Os dados dos sensores são enviados via **MQTT** para o FIWARE.
- Cada leitura inclui um **timestamp** (carimbo de tempo) para fins de rastreabilidade.
- A **interface** para acessar as informações deve estar em conformidade com o padrão **NGSI**.

---

## 🧪 Montagem do Circuito

Ambiente simulado do circuito 👉 [**Clique aqui**](https://wokwi.com/projects/443101891666438145)

---


