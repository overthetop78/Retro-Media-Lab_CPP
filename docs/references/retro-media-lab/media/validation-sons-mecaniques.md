# Catalogue des sons mécaniques

## Portage et organisation

Les 30 définitions C# de Constants/MechanicalAudio sont regroupées dans des tables constantes typées (opérations, échantillons, profils), séparées du service de sélection. Les contrats ne dépendent ni du GUI ni des périphériques audio. Aucun nouveau moteur sonore ou DLL supplémentaire n’est introduit par ce bloc.

25 profils : 1 bande, 18 disquettes, 5 disques durs et 1 optique. Les 1 759 FLAC de production sont copiés avec leurs chemins relatifs ; leur SHA256 a été comparé à la source C# pendant la copie. La règle CMake installe la banque dans Assets/MechanicalAudio ; l’installation complète n’a pas été exécutée dans ce contrôle.

La sélection conserve l’ordre C#, avec priorité au sens, puis à la distance et au cylindre ; les égalités conservent le premier élément. Le repli vers le profil de base intervient seulement si l’opération manque dans le profil demandé. Les calculs de distance utilisent un entier 64 bits.

## Vérifications effectuées

Compilation Debug de MediaEngine réussie après conversion des données en tables constexpr. Contrôle ciblé réussi : ordre des 25 profils, unicité et existence des 1 759 fichiers, sélection, égalités, profils par défaut, repli et opération de démarrage des bandes. Audit réussi : 1 383 fichiers C++, 56 clés d’erreur, 29 langues, aucune valeur applicative hors constantes nommées. Les premiers agrégats générés n’étaient pas conformes à cet audit : ils ont été remplacés par les tables constexpr, sans relâcher le contrôle.

Le banc, son importateur et leurs fichiers temporaires ont été supprimés après utilisation. Aucun média de test n’est conservé ; la banque FLAC est une ressource de production.

## Taille et lecture

Somme des fichiers FLAC : 48 870 902 octets, soit 46,61 Mio. Les champs STREAMINFO indiquent un total de 104 911 966 octets PCM ; avec un en-tête WAV PCM minimal de 44 octets par fichier, cela représente 104 989 362 octets, soit 100,13 Mio et 2,15 fois la taille FLAC. Estimation obtenue sans conversion et sans modifier les fichiers ; des métadonnées WAV supplémentaires augmenteraient légèrement ce total.

Le C# conserve les PCM décodés par son et fréquence. Son cache des durées est distinct et peut entraîner un décodage initial supplémentaire. Ce bloc C++ ne porte pas encore le décodage, ces caches, le rendu, les alias synthétiques ou MediaDriveMechanics. Aucune mesure de latence ou de coût CPU n’a été effectuée ; le seul catalogue compilé ne permet pas d’en déduire les performances de l’émulation.

Références techniques consultées : [FLAC, caractéristiques](https://xiph.org/flac/features.html), [format FLAC et STREAMINFO](https://www.rfc-editor.org/rfc/rfc9639.html), [Microsoft ADPCM](https://learn.microsoft.com/en-us/windows/win32/xaudio2/adpcm-overview). Le PCM est restitué sans pertes par FLAC ; cela ne signifie pas que le décodage est gratuit. Le WAV PCM évite la décompression mais double ici approximativement la taille sur disque.
