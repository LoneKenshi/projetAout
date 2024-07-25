# projetAout
Projet de seconde session pour le cours de développement 2024


serveur:
- serveur attends qu'un client demande un fichier
- si ce fichier existe:
  - calculer son hash
  - envoyer le fichier
- si ce fichier n'existe pas:
  - envoyer un code pour le dire au client

client:
1) client etablis une connection avec un serveur
2) client communique un nom de fichier (qui devrait se trouver sur ce serveur)
3) si ce fichier existe, le client recoit le code "OK"
  5) le client recoit son hash
  6) le client recoit ce fichier
  7) le client calcule son hash et le compare au hash recu
  8) si les deux hash sont identiques:
    9) fin de connection
  10) si les deux hash sont differents:
    11) on revient a l'etape 3) # TODO, prevoire le cas au on est dans une boucle
12) si le fichier n'existe pas, le client recoit le code "NOK" et termine la connection


liste des fonctions:
- fonction pour hacher un fichier
- fonction pour comparer deux hash
- fonction pour verifier si un fichier existe