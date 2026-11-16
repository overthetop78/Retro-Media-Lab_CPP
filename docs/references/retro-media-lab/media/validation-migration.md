# Préparation et validation des migrations

Référence : `F:/GW GUI/src/RetroMediaLab.MediaFileSystems/Migration/` (consultée sans modification), ainsi que `Interfaces/Migration/IMigrationNamePolicy.cs`.

Le contrat existant `Migration.h` est étendu, sans nouvelle DLL ni second modèle d’entrées. Le planificateur copie les contenus et construit les chemins des descendants. Les capacités fournies par l’appelant définissent les noms, limites, types et métadonnées acceptés. Les politiques de noms sont des fonctions injectables, équivalentes au contrat C#.

La validation conserve l’ordre C# des groupes de collisions et des pertes. Les collisions concernent chaque dossier, avec comparaison ordinale sensible ou insensible à la casse. La longueur des noms compte les unités UTF-16 comme .NET ; les caractères de contrôle sont traités selon leur catégorie Unicode. Les pertes de contenu, noms, taille, types et métadonnées invalides bloquent toujours. Seules date de modification, commentaire et attributs peuvent être abandonnés après consentement. Le réducteur retourne une copie indépendante et ne modifie jamais le plan source.

Contrôle ciblé Debug réussi : copie indépendante des octets, chemins imbriqués, trois pertes de métadonnées, date au format aller-retour avec fraction de sept chiffres et décalage négatif, refus sans consentement, acceptation explicite, réduction, préservation des métadonnées prises en charge, contenu manquant, collision Ä/ä, comparaison sensible à la casse, longueur UTF-16 d’un caractère supplémentaire, contrôle DEL, politique de noms, nom de volume, métadonnées invalides, taille excessive et erreurs de paramètres.

Compilation Debug de MediaFileSystems réussie. Les quatre erreurs ont été générées avec Argos dans les 29 langues, relues et corrigées (notamment négations japonaises/coréennes, textes arabes restés anglais et chinois traditionnel). Les 30 catalogues DLL ont été compilés. Audit : 1366 fichiers C++, 56 clés App, 29 langues, conforme. Le contrôle temporaire et ses sorties ont été supprimés après utilisation.

Ce bloc ne porte pas encore le catalogue concret des capacités ni `FileSystemMigrationService.Inject` : raccordement aux écrivains, contrôle de l’image vierge et conservation de la géométrie restent à réaliser. L’inventaire conserve ces entrées non portées.
