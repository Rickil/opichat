# OPIChat

OPIChat est une application de messagerie instantanée inspirée d'IRC, développée en C. Elle comprend un serveur et un client, permettant des communications en temps réel via des sockets.([GitHub][1])

## Fonctionnalités

* Architecture client/serveur simple.
* Utilisation de sockets pour la communication réseau.
* Gestion des connexions multiples via `epoll`.
* Protocoles de communication personnalisés.
* Interface en ligne de commande pour le client.

## Structure du projet

* `basic_client/` : Code source du client de base.
* `basic_server/` : Code source du serveur de base.
* `epoll_server/` : Implémentation du serveur utilisant `epoll` pour la gestion des connexions multiples.
* `src/` : Fichiers sources communs.
* `tests/` : Tests unitaires et d'intégration.
* `Makefile` : Script de compilation du projet.([GitHub][1])

## Compilation

Assurez-vous d'avoir les dépendances suivantes installées :

* `gcc`
* `make`([GitHub][1])

Clonez le dépôt et compilez le projet :

```bash
git clone https://github.com/Rickil/opichat.git
cd opichat
make
```



Les exécutables seront générés dans les répertoires correspondants.

## Utilisation

### Serveur

Lancez le serveur avec la commande suivante :

```bash
./opichat_server <ip> <port>
```



* `<ip>` : Adresse IP sur laquelle le serveur écoutera.
* `<port>` : Port sur lequel le serveur écoutera.([GitHub][1])

Exemple :

```bash
./opichat_server 127.0.0.1 8080
```



### Client

Lancez le client avec la commande suivante :

```bash
./opichat_client <ip> <port>
```



* `<ip>` : Adresse IP du serveur.
* `<port>` : Port sur lequel le serveur écoute.

Exemple :

```bash
./opichat_client 127.0.0.1 8080
```



Une fois connecté, le client vous invite à entrer des commandes. Suivez les instructions à l'écran pour interagir avec le serveur.
