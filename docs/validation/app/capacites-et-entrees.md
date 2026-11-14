# Capacités optiques, audio et séparation des services

Validation Windows du 14 novembre 2026.

Infrastructure.dll contient la détection matérielle Windows, les outils Greaseweazle/KryoFlux, l'extraction d'archives et la gestion des journaux/préférences/réglages. La cible ne dépend ni d'App ni de Qt Widgets. App conserve les vues, l'état de présentation et le raccordement des commandes aux services. Le test optique asynchrone et son nettoyage sont possédés par Infrastructure.

Le scan audio retourne huit entrées sur le PC de validation : trois entrées ligne Realtek débranchées, le mixage stéréo désactivé, deux entrées micro analogiques débranchées et les deux micros USB actifs. Le modèle conserve la disponibilité réelle ; ajouter une entrée désactivée ou débranchée ne la transforme pas en entrée active. Les entrées ligne sont présentées en premier, les autres types restent accessibles. Les interfaces USB fournissant une entrée audio Windows sont couvertes par cette énumération ; aucune reconnaissance MIDI comme source audio.

Interrogations matérielles réelles réussies, sans écriture ni extraction de disque :

| Lecteur | Lecture CD | DVD | BD | C2 annoncés | CD-Text annoncé |
| --- | --- | --- | --- | --- | --- |
| ASUS BW-16D1HT | Oui | Oui | Oui | Oui | Oui |
| HL-DT-ST DVDRAM GH24NSD5 | Oui | Oui | Non | Oui | Oui |

Ces informations proviennent de MMC GET_CONFIGURATION. Elles ne constituent pas une mesure de fiabilité C2, un test de sous-canaux complets, une validation GD-ROM ni une extraction bit à bit réussie. L'interface indique explicitement cette distinction.

Le banc vérifie le décodage big-endian, les descripteurs tronqués/invalides, la fin d'une requête asynchrone en erreur, la destruction du service avec requête en cours, les états audio dans l'interface, le résultat du bouton de test et la conservation des résultats lors d'une reconstruction de la présentation. Les textes nouveaux sont générés par Argos puis relus/corrigés, notamment les faux sens « essai routier » et « entrée de texte ».

Installation de validation séparée : la fenêtre C++ Debug habituelle reste ouverte à la demande de l'utilisateur. Sources de tests, captures et installation temporaire supprimées après validation ; aucun test dans le workflow GitHub.
