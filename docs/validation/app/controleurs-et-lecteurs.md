# Panneau matériel repris depuis le C#

Validation Windows du 14 novembre 2026. La version à formulaire générique est remplacée par les vues décrites ici.

## Interface

Les six sous-onglets sont conservés. Disquettes reprend les cartes C# : port, lecteur numéroté, taille, densité, vitesse, disponibilité, état de configuration et commandes enregistrer/oubli sur chaque carte. La liste est limitée en hauteur avec défilement interne si nécessaire. L'ajout part d'un contrôleur connu ; pas de champ COM, identifiant USB ou interface fictive à saisir. Deux lecteurs Greaseweazle maximum, sélections A/B conservées. Les réglages du lecteur sont sauvegardés explicitement comme en C#.

Les autres catégories proposent les périphériques détectés par Windows, un sélecteur d'ajout et des cartes des appareils enregistrés. Les identifiants techniques sont conservés dans le modèle et ne sont pas des champs de saisie. Audio expose les réglages de l'entrée sélectionnée. Optique indique que la compatibilité d'extraction reste non vérifiée. Aucun appareil ne peut être ajouté par un faux formulaire générique. Les cartouches n'ont pas encore de pilote de détection compatible et leur commande d'ajout reste désactivée sans périphérique détecté. Les configurations déjà mémorisées sont conservées.

## Greaseweazle

Détection/processus isolés dans GreaseweazleDiscovery ; la vue ne construit plus un moteur de détection générique. Confirmation par gw info, délai limité, lecture des informations et journalisation, arrêt et attente du processus sur destruction normale ou par exception.

Le panneau des outils reprend les quatre commandes C# : rechercher gw.exe, rechercher une mise à jour, télécharger la dernière version et utiliser le chemin précédent. Le téléchargement consulte l'API officielle GitHub, demande confirmation dans l'application, télécharge et vérifie SHA-256 lorsque fourni, puis installe la version dans le dossier Greaseweazle géré. Il ne lance pas le logiciel téléchargé. Le ZIP est traité en C++ avec libarchive intégré statiquement, sans PowerShell ni outil d'extraction externe. Une installation préexistante complète est conservée ; un dossier préexistant incomplet n'est pas supprimé automatiquement. Les fichiers temporaires sont gérés par RAII.

Les fichiers ZIP avec chemins absolus, traversée de répertoires, liens symboliques ou contenu incompatible sont rejetés. L'installation est préparée dans un dossier temporaire voisin puis promue une fois complète. Les ressources réseau sont annulées et libérées à la fermeture. Les erreurs et confirmations, y compris Oui/Non, viennent des catalogues des 29 langues.

## Vérifications

- Debug et Release construits avec Qt Network et libarchive ; DLL Qt Network rangée dans lib/Qt6, licence libarchive livrée dans licenses/libarchive.
- Cartes de lecteurs, sauvegarde sur la carte, ajout du second lecteur et limite de deux, six sous-onglets et absence des anciens champs génériques vérifiés.
- Commandes d'ajout désactivées en l'absence de périphérique réellement détecté ; aucun accès aux médias physiques effectué par le banc.
- Formateur gw info vérifié sur sorties de test ; processus de test arrêté sur exception pendant la destruction du service.
- ZIP valide imbriqué installé et normalisé ; archives sans gw.exe, chemins absolus et traversée rejetés, sans installation partielle ni reste de dossier temporaire.
- Consultation de la publication officielle puis téléchargement et installation réels dans un dossier temporaire réussis. Le gw.exe téléchargé n'a pas été exécuté.
- Rendu Disquettes inspecté à 1100 × 640 avec la police du thème et une échelle de 150 %. Cartes et commandes des outils visibles.
- Commandes des outils vérifiées dans les 29 langues ; clés, paramètres et invariants vérifiés, ainsi que l'audit des constantes C++ et git diff --check.

Les tests, captures, archives et sources temporaires de récupération ont été supprimés après validation. Aucun test ajouté au workflow GitHub. L'acquisition physique des médias reste le travail des services médias à porter ; elle n'est pas simulée par les préférences.

## Validation du selecteur et des archives multiformats

- Associations Greaseweazle/KryoFlux separees meme avec un identifiant identique, deux lecteurs maximum par controleur, A/B pour GW, 0/1 physiques pour KryoFlux, refus des doublons et conservation du numero 1 apres suppression de 0.
- Persistance JSON et compatibilite des anciennes preferences ; ajout et sauvegarde des deux lecteurs KryoFlux par les commandes de la vue verifies.
- Reglages non enregistres conserves pendant changement de controleur et reconstruction du theme, sans sauvegarde implicite ; chemins GW/DTC independants ; changement fr-FR/en-US.
- Extraction ZIP, 7z, TAR et TAR compresses gzip/bzip2/xz/Zstandard/LZ4 verifiee. Lecture des donnees RAR4/RAR5 sur fixtures officielles libarchive. Traverses, chemins absolus Windows/Unix, liens et archives invalides rejetes, aucune installation partielle apres erreur.
- Rendu KryoFlux avec deux lecteurs inspecte a 1100 x 640, echelle 150 %, police reelle du theme.
- Aucun peripherique physique ni calibrage utilise. Compilation Windows uniquement ; Linux/macOS non compiles ici.

Les sources de recuperation, le banc temporaire, les captures et archives de verification sont supprimes apres utilisation. Sources integrees et licences conservees dans third_party/.
