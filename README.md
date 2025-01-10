# P1RV_Piano
Sujet8: Instrument de musique virtuel
Instructions d'utilisation

Pré-requis avant l’exécution
Assurez-vous que le fichier glut32.dll est présent dans le dossier du programme avant de lancer l’application.

Contrôle via la souris et le clavier
Le piano virtuel peut être contrôlé à la fois à l’aide de la souris et du clavier.


Correspondance des touches de clavier
Le clavier est mappé en fonction de la disposition américaine (QWERTY).Pour les utilisateurs de claviers français (AZERTY), la correspondance reste logique :
La ligne supérieure de lettres (A-Z) correspond aux touches noires.
La deuxième ligne de lettres correspond aux touches blanches.
La troisième ligne commence à la touche C pour les touches blanches supplémentaires.
Les positions des touches restent cohérentes et n'affectent pas l'utilisation.

Changement de projection
Appuyez sur la barre d’espace pour basculer entre la projection orthogonale et la projection en perspective.
Cependant, la projection orthogonale est principalement utilisée pour ce projet.

Mode tutoriel et projection Perspective
Le mode tutoriel a été conçu pour offrir une expérience optimisée pour une projection orthogonale. Cette configuration garantit une meilleure lisibilité des blocs descendants et un alignement précis avec les touches du piano. Pour une expérience optimale, il est recommandé d'utiliser le mode de projection orthogonale pendant l'utilisation du tutoriel.

Emplacement de l’enregistrement
Les fichiers d'enregistrement audio sont générés automatiquement dans le dossier du projet, au format WAV. 

Sélection et redémarrage des morceaux
Trois morceaux sont intégrés dans le système.Vous pouvez les sélectionner en appuyant sur les touches 1, 2 ou 3.
Le terminal affiche le morceau actuellement sélectionné ainsi que l’état de isDrop (indiquant si les blocs descendants sont activés).
En appuyant deux fois sur la même touche (par exemple, 1), le morceau correspondant redémarrera.
