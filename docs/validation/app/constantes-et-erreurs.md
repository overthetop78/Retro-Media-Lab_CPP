# Inclusions, constantes et diagnostics — 14 novembre 2026

## Rangement

Les constantes techniques sont réparties entre AppConstants, ThemeConstants (RetroMediaLab), TranslationConstants, LauncherConstants, PlatformConstants et les constantes communes de diagnostic et de sortie de processus. ErrorKeys rassemble les clés d'erreur partagées. Les coordonnées et dimensions des tracés d'icônes sont dans icons.json, embarqué dans la DLL du thème ; le lecteur valide les commandes et leurs arguments.

Les libellés et erreurs sont dans les catalogues. Constants est la base anglaise, équivalente au rôle de 00-Base dans le C#. Le fichier errors.json contient 53 clés, déclinées dans les 29 langues. Le secours de démarrage est généré depuis ces mêmes fichiers, sans seconde base maintenue manuellement. Il permet de diagnostiquer une panne de chargement des catalogues ou du GUI ; les erreurs natives du lanceur emploient la langue du système et les erreurs applicatives la langue sélectionnée.

Les clés et paramètres techniques sont des constantes, les textes traduisibles des données de catalogue. Les littéraux de syntaxe (`extern "C"`, inclusions, zéro/un structurels, booléens, nullptr) ne sont pas des paramètres de présentation ou de configuration. Le script scripts/quality/audit_literals.py contrôle cette convention sans être ajouté au workflow GitHub. Il détecte les textes/valeurs dans le code d'exécution ainsi que les clés d'erreur ou paramètres de traduction manquants.

Les messages des scripts de développement restent hors traduction, conformément au choix utilisateur. Aucun installateur n'existe dans le dépôt : sa traduction devra être embarquée dans le futur programme d'installation.

## Vérifications

- Toutes les unités C++ Windows sont présentes dans compile_commands.json et les répertoires d'inclusion existent. Les sources communes, App, thème, Translations, plateforme et lanceur sont compilés ensemble.
- Le programme de contrôle compare toutes les erreurs des catalogues normaux avec leur secours embarqué dans les 29 langues ; les messages et paramètres correspondent.
- Une erreur de configuration émise par la DLL de thème, une langue invalide dans les préférences et une impossibilité de charger les catalogues produisent des clés résolues en français. Le contexte de chemin UTF-8 est conservé. Une clé inconnue a un diagnostic général traduit.
- Un second exécutable de contrôle, sans Qt ni DLL de catalogue, résout les 29 langues et le repli anglais. Les erreurs Windows gardent leur code technique et leur contexte dans un message traduit.
- La fenêtre de préférences est rendue après extraction des icônes ; les données de session sont conservées. Les fenêtres, vues, abonnements et dossiers temporaires sont possédés et libérés, y compris sur exception.
- Relecture des 29 fichiers Argos et corrections des sorties manifestement erronées, des résidus anglais, des balises Name/Comment et des caractères chinois traditionnels. Cette relecture ne certifie pas une validation par des locuteurs natifs des 29 langues.

Les contrôles utilisent uniquement des dossiers temporaires ; ils ne modifient pas les préférences personnelles ni la langue de Windows. Les programmes de vérification, images et journaux sont supprimés après utilisation.

## Résultat final

- Audit réussi des 67 fichiers C++ et des 53 erreurs dans les 29 langues, y compris invariants et paramètres. Zéro erreur de compilation ou avertissement C++ relevé dans les builds finaux.
- La solution à la racine a été compilée via MSBuild en Debug et Release x64. Son projet liste tous les fichiers sources présents ; chacun des chemins déclarés existe. La base CMake couvre les 27 unités Windows du dépôt, avec chemins d'inclusion existants. Les entrées Unix et leur lanceur direct restent à compiler sur ces systèmes.
- Les balises système fr-Latn-FR, zh-Hant-HK et pt-Latn-BR sont correctement résolues ; une langue inconnue revient à en-US. loadOrCreate enregistre la langue dès la première lecture d'un nouveau fichier. Une nouvelle détection avec une autre langue ne remplace pas celle déjà sauvegardée.
- Le contrôle autonome de secours n'a pour dépendance DLL que KERNEL32 ; aucune dépendance Qt ou catalogue. Il vérifie le français, le contexte UTF-8 et le repli anglais.
- Avant que les préférences aient pu être lues, les diagnostics précoces utilisent la langue système ; après lecture, les erreurs applicatives utilisent la langue enregistrée. Les données de diagnostic sont copiées avant destruction des services et affichées après leur nettoyage.
- Les sources de vérification, exécutables, captures et journaux sous build/verification-constants ont été supprimés. Le contrôle statique autonome reste dans scripts/quality, sans intégration au workflow GitHub.
