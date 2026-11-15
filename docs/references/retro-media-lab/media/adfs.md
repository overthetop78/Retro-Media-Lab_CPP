# ADFS — exploration et structures initiales

Référence directe : les onze fichiers `FileSystems/Acorn/Adfs/` et `Utilities/AcornFileSystemTime.cs` du dépôt C# `F:/GW GUI/src/RetroMediaLab.MediaFileSystems`. Les douze empreintes SHA-256 sont inchangées. Le codec de noms était déjà porté dans FileCore ; les autres comportements ont été repris dans ce bloc.

## Organisation

MediaFileSystems possède le parcours ADFS. `Reader.cpp` choisit le résolveur FileCore ; `Directory.cpp` valide les catalogues et parcourt leurs descendants ; `File.cpp` lit les contenus ; `BlockView.cpp` adapte un volume adressable par octets ; `Formatter.cpp` produit les structures initiales BBC et D ; `Time.cpp` décode les dates et descriptions Acorn. Les contrats sont regroupés dans `Adfs.h`. Les positions de champs des cartes déjà définies dans FileCore sont réutilisées, plutôt que recopiées dans les constantes ADFS.

Les variantes BBC emploient des secteurs de 256 octets, deux secteurs de carte et une racine à l'unité 2. Le format L adapte l'ordre par face aux pistes entrelacées. Les variantes à secteurs de 1024 octets essaient la nouvelle carte FileCore avant le repli sur l'ancienne carte et sa racine à l'unité 4. Le choix est structurel, comme dans le C# ; il ne repose pas uniquement sur le nom du format.

## Répertoires et fichiers

Les répertoires Hugo/Nick doivent avoir des signatures initiale/finale identiques et les deux numéros de séquence concordants. Le lecteur conserve les 47 entrées BBC ou 77 entrées Archimedes, les attributs BBC portés par les bits hauts des quatre premiers caractères, les attributs Archimedes, noms sur sept bits, adresses et longueurs. Les dossiers sont triés avant les fichiers, puis les noms selon la comparaison ordinale ASCII insensible à la casse. Cycles, références répétées et limite de profondeur 64 suivent les décisions C#.

Les adresses sont résolues à chaque portion lue, ce qui conserve les fragments séparés et partagés d'une nouvelle carte. Un fichier incomplet reçoit un contenu absent et des métadonnées invalides ; il n'est pas présenté comme un fichier vide. Un fichier de longueur zéro garde une collection vide, même si son adresse est invalide, avec le drapeau de métadonnées correspondant.

Les dates RISC OS utilisent le marqueur des champs load/execute et des centisecondes depuis le 1er janvier 1900 UTC. Les descriptions techniques et le type RISC OS sont conservés à l'identique dans des constantes, avec formatage hexadécimal indépendant de la locale.

## Accès par blocs et fiabilité

La vue de volume emprunte le BlockMedia uniquement pendant l'appel synchrone. Elle lit un secteur de 256 octets à la demande et applique les bornes start/length avant tout accès ; aucun chargement intégral du disque parent. Le volume minimal est de sept secteurs, sa longueur doit être multiple de 256 et son nombre de secteurs tenir dans Int32, comme dans le C#. Les échecs d'accès aux données sont convertis en secteurs indisponibles ; l'annulation reste propagée.

Les erreurs et avertissements sont des codes/paramètres du domaine MediaFileSystems. L'échec d'un sous-répertoire conserve un contexte de nom séparé du diagnostic, au lieu d'insérer une exception anglaise dans le texte. Les ressources possédées et les résultats intermédiaires sont libérés par RAII sur succès, exception et annulation.

Renforcements explicites par rapport au C# : les drapeaux d'intégrité invalides des anciennes cartes, répertoires et fichiers ne sont plus ignorés ; les octets de fichier restent disponibles, mais leur entrée est marquée invalide et reçoit un diagnostic. Une carte ancienne ou racine concernée rend l'espace libre non fiable. Les nouvelles cartes conservent les contrôles de sommes de contrôle FileCore. Une longueur de fichier supérieure à toute la capacité est refusée avant allocation ; le résultat reste contenu absent/métadonnées invalides, mais le diagnostic peut apparaître dès l'offset zéro au lieu d'après les premières lectures.

## Initialisation et limites

Les en-têtes BBC contiennent les deux cartes, leur somme de contrôle et le répertoire Hugo ; l'en-tête D contient la carte et le répertoire Nick avec son octet de contrôle. Leur construction n'écrit aucun fichier et ne formate aucun périphérique. Ce ne sont pas des writers généraux de fichiers ADFS.

Les [conteneurs ADF Acorn de MediaEngine](adf.md) sont désormais portés ; la reconnaissance/enregistrement générale et les contrats de documents applicatifs restent à raccorder. Les surcharges sectorielles et BlockMedia fournissent les algorithmes nécessaires ; aucun accès direct d'App à MediaFileSystems n'est ajouté.
