# Images brutes IBM/MSX — validation

Date : 15 novembre 2026. Bloc 04 du portage complet, objectif global toujours incomplet.

- Compilation et installation Debug/Release réussies sans erreur ni avertissement C++.
- Même banc exécuté en Debug et Release : 120 393 contrôles réussis.
- 87 images C# de référence : onze profils IBM, longueurs nulles/non alignées/inconnues, BPB couvrant cylindres/têtes/secteurs standards et atypiques, bornes de détection et descripteurs MSX. Géométries détectées IBM, boot seul et reconnaissance MSX/ambiguïtés identiques.
- Catalogues ordonnés IBM/MSX égaux au C# ; les 256 valeurs de descripteur FAT historique sont comparées.
- Quinze écritures C++ comparées aux writers C# originaux : onze profils IBM (DMF inclus) et quatre MSX. SHA-256 des fichiers complets identiques.
- Les images lues depuis les fichiers sont vérifiées secteur par secteur : adresse CHS, ordre, contenu et SHA-256 global. Un compteur de source confirme une seule lecture à l’ouverture, puis une lecture sur demande pour le secteur consulté.
- Trajets automatiques IBM et MSX : fichier → détection → secteurs → FAT12 → analyse des entrées, nom/texte extrait et identifiants de classification vérifiés.
- Publication atomique : succès et remplacement, exception injectée après écriture partielle, annulation pendant production, secteur absent ou tronqué, géométrie et profil incorrects, parents à créer, cible invalide et chemin Unicode. La destination précédente est conservée après les échecs et aucun temporaire ne subsiste.
- Annulation vérifiée après lecture du boot et avant publication. IBM scan, contrôle de format dans la primitive générique et intégrité brute inconnue vérifiés.
- 348 résolutions de diagnostics dans les DLL de langues égales aux JSON (12 clés × 29 langues). Relecture Argos, corrections de sens et de paramètres, réutilisation des traductions d’annulation de stockage existantes. Audit des constantes et diagnostics et git diff --check réussis.

Le C# de référence est compilé directement par liens en lecture seule pour les détecteurs, catalogues, modèles et writers. Son transport de fichier est remplacé dans le banc par un adaptateur minimal File.Create ; les algorithmes de géométrie/écriture sont ceux du projet. La fiabilité de l’écriture atomique C++ est vérifiée séparément par l’état réel des fichiers après exception/annulation.

SHA-256 du manifest C# (géométries et sorties) : `1d8971b1ee9bfea918e622c3953da9ce7ae6e3abb2f5ec4777ff6c26253223a5`.

22 fichiers sources C# concernés sont revérifiés identiques à leur empreinte d’inventaire. Banc et artefacts temporaires supprimés après validation ; aucun média matériel n’est écrit et aucun test n’est ajouté au workflow GitHub.

Les évolutions volontaires (source paresseuse, intégrité non supposée et capacité d’écriture MSX contrôlée) et le périmètre restant sont détaillés dans [la référence du bloc](../../references/retro-media-lab/media/images-brutes-ibm-msx.md).
