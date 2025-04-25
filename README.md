# Proyecto 1 - Telemática

## Equipo de Trabajo
- Miguel Angel Arcila Cifuentes
- Alejandro Carmona Salinas
- David Restrepo Aristizabal

## Introducción

Este proyecto implementa un sistema de chat en tiempo real basado en el modelo cliente-servidor, programado en C y utilizando la API de Sockets de Berkeley sobre TCP/IP. El servidor gestiona múltiples conexiones de clientes de forma concurrente (POSIX Threads) y retransmite los mensajes recibidos a todos los clientes conectados. El objetivo es comprender y aplicar conceptos de programación en red, concurrencia y despliegue en entorno realista (VirtualBox y AWS Academy).

## Desarrollo

La estrategia de desarrollo fue dividir el proyecto en pasos claros y hacer pruebas constantes en cada etapa. Primero diseñamos y probamos el servidor por separado, creando el socket, manejando conexiones y retransmisión de mensajes. Luego implementamos el cliente y comprobamos que pudiera enviar y recibir datos correctamente. A continuación añadimos concurrencia con hilos para soportar varios usuarios al mismo tiempo y un sistema de logs para registrar lo que ocurre. Finalmente, validamos todo en un entorno local (Ubuntu en VirtualBox) y desplegamos el servidor en AWS para garantizar que funcionara de verdad en la nube. Este es un pequeño paso a paso de como se abordo cada parte del proyecto y se muestra la lógica detras del servidor y del cliente.

### Servidor

1. **Creación del socket TCP**  
   ```c
   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   bind(listenfd, …, htons(PORT));
   listen(listenfd, 10);
   ```
2. **Aceptación de clientes**  
   - Bucle infinito de `accept()`.  
   - Por cada conexión se crea un hilo (`pthread_create`) que ejecuta `handle_client()`.

3. **Concurrencia y sincronización**  
   - Array global `clients[MAX_CLIENTS]` guardando punteros a estructuras `client_t`.  
   - Mutex (`pthread_mutex_t`) para proteger alta/baja de clientes y envío de mensajes.

4. **Broadcast de mensajes**  
   - Cada hilo recibe con `recv()`, formatea `"usuario: texto"` y llama a `broadcast_message()`, que recorre `clients[]` y hace `send()` a cada socket.

5. **Gestión de usuario y desconexión**  
   - Al conectar, el servidor envía el prompt `"Usuario: "` y almacena el nombre.  
   - Al desconectarse, elimina al cliente del array, cierra el socket y notifica al resto.

6. **Registro de actividad**  
   - Función `log_event()` anexa en `logs/chat.log` conexión, mensajes y desconexiones con timestamp.

### Cliente

1. **Creación y conexión del socket**  
   ```c
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   connect(sockfd, …, puerto);
   ```
2. **Ingreso de usuario**  
   - Recibe prompt `"Usuario: "`, lee con `fgets()` y envía el nombre.

3. **Concurrencia cliente**  
   - Hilo de recepción (`recv_handler`) que imprime todo lo que llega.  
   - Hebra principal lee de `stdin` y envía líneas con `send()`.  
   - Comando especial `exit` para cerrar sesión limpiamente.

### Despliegue

- **VirtualBox (Ubuntu VM)**: entorno de desarrollo y pruebas locales.  
- **AWS EC2 (Ubuntu Server 24.04 LTS, t2.micro)**: servidor en la nube, con Security Group abierto a SSH (22) y chat (9000).  
- SSH con clave importada y `tmux` para mantener el servidor corriendo en background.

## Aspectos Logrados y Aprendizajes

• Llevar la teoría de la clase a la práctica
• Comunicación en tiempo real entre múltiples clientes    
• Uso de Berkeley Sockets sobre TCP/IP                              
• Manejo de login y logout con nombres de usuario       
• Registro detallado en un log              
• Despliegue en VM local y en AWS EC2
• Configuración de puertos y reglas de seguridad

## ¿Cómo ejecutar el código?
### 1. Prerrequisitos

Instalar:
- **Git**  
- **GCC**
- **Make**  


### 2. Clonar el repositorio

```bash
git clone https://github.com/Miguel107/chat-sockets-c.git
cd chat-sockets-c
```

### 3. Preparar carpeta de logs

El servidor escribe en `logs/chat.log`. Crea la carpeta si no existe:

```bash
mkdir -p logs
```

### 4. Compilar

```bash
make
```

### 5. Ejecutar el servidor

En una terminal nueva:

```bash
./bin/server
```

Deberías ver:
```
Servidor escuchando en puerto 9000
```

### 6. Ejecutar clientes

Abre una o más terminales adicionales y lanza:

```bash
./bin/client 127.0.0.1 9000
```

- Cuando aparezca `Usuario: `, escribe tu nombre y pulsa **Enter**.

## 7. Probar la comunicación

- Escribe mensajes en cualquier cliente: se retransmitirán a todos.  
- Revisa `logs/chat.log` para ver conexiones, mensajes y desconexiones.

### 8. Detener todo

1. En cada cliente, escribe:
   ```text
   exit
   ```
2. En la terminal del servidor, presiona:
   ```
   Ctrl + C
   ```

## Conclusiones

Este proyecto nos ha permitido adquirir experiencia práctica en:

- Programación de sockets en C usando la API de Berkeley.  
- Manejo de concurrencia y sincronización con hilos.  
- Diseño de una arquitectura cliente‑servidor robusta.  
- Despliegue de aplicaciones de red en entornos virtualizados y en la nube (AWS Academy).  

Los principales retos superados incluyeron la sincronización de múltiples conexiones, la correcta gestión de desconexiones y la producción de logs confiables. De igual manera, acercarnos cada vez más a entornos realistas como AWS nos dió unas bases más claras y robustas de las diversas aplicaciones de los temas que hemos visto en clase.

