# Validation des cadences et profils — 15 novembre 2026

Vingt et une sources C# de catalogues/constantes sont vérifiées contre l’inventaire, sans modification des originaux. Empreinte du manifeste : `2e4a344651ad45aea2bc70e481dc8ff385d718eb12b618d5fc9edec6e1f8fb60`.

Le banc compile directement SectorImageTrackTimingCatalog, TrackEncodingTimings, TrackEncodingDefaults, RareTrackEncodingProfiles, RareDiskImageFormatIds et FluxCodecIds. Les 21 liaisons de constantes de formats nécessaires au catalogue standard sont extraites des déclarations C# originales ; aucun algorithme de cadence n’est réécrit dans le banc de référence.

1 559 cas comparent cadence, durée de révolution et présence/contenu du profil rare. Ils couvrent les formats déclarés, les exceptions de densité, les préfixes, les inconnus, les espaces conservés, les suffixes, la casse, U+FEFF initial/final et les caractères Unicode pouvant être confondus avec des lettres ASCII. Chaque profil reconnu est recherché dans le registre réel d’encodeurs C++ : 42 recherches pour les quatorze profils et leurs trois variantes de casse.

La première comparaison a trouvé sept écarts : la comparaison Qt assimilait des caractères non ASCII aux identifiants ASCII du catalogue. Le contrôle a été remplacé par une comparaison ASCII explicite. La première compilation a aussi révélé un nom d’espace incorrect Emu au lieu d’EmuFm ; il est corrigé.

- Construction et déploiement complets Debug/Release réussis.
- Chaque configuration : **1 601 contrôles, 0 différence, 1 559 cas, 42 recherches de profils**.
- Audits : **548 fichiers C++**, **536 fichiers pour les frontières**, **0 violation** ; git diff --check réussi.
- Aucun nouveau message d’erreur à traduire dans ce bloc.
- Le banc temporaire et ses journaux sont retirés après validation ; aucun test dans le workflow GitHub.

La transformation des blocs sectoriels et les opérations de reconstruction d’images complètes constituent la suite du portage ; ces résultats ne les valident pas encore.
