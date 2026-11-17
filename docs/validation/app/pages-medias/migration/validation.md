# Migration de fichiers

Référence : FileMigrationCoordinator, FileMigrationWindow et Migration.resx du projet C# (consultés en lecture seule).

Le bouton de Conversion ouvre un dialogue possédant une session Infrastructure. Le service MediaEngine existant prépare les destinations et écrit les images ; App affiche la source, la destination, le tableau des pertes et leur acceptation. Une perte bloquante interdit la migration, une perte de métadonnées exige l’acceptation explicite. Le fichier source est protégé, y compris si les chemins désignent le même fichier, et un fichier de destination existant exige confirmation.

Vérification ciblée réussie avec une petite image FAT12 créée temporairement : chargement, choix de destination Atari ST, écriture réelle, protection de la source, refus d’écraser sans confirmation, annulation du chargement, contenu absent bloquant et acceptation des pertes de commentaire. Le véritable module du thème affiche les onze destinations et le tableau des pertes. Le dialogue conserve session et géométrie après changement fr-FR/en-US et reconstruction du thème. Les boutons ne présentent aucune clé de traduction manquante.

La capture dialog.png utilise le rendu Qt hors écran. Les programmes, images d’entrée/sortie et artefacts de vérification sont supprimés après utilisation. Aucun matériel n’a été sollicité.
