# Préparation et validation des migrations

Référence : `F:/GW GUI/src/RetroMediaLab.MediaFileSystems/Migration/` (consultée sans modification), ainsi que `Interfaces/Migration/IMigrationNamePolicy.cs`.

Le contrat existant `Migration.h` est étendu, sans nouvelle DLL ni second modèle d’entrées. Le planificateur copie les contenus et construit les chemins des descendants. Les capacités fournies par l’appelant définissent les noms, limites, types et métadonnées acceptés. Les politiques de noms sont des fonctions injectables, équivalentes au contrat C#.

La validation conserve l’ordre C# des groupes de collisions et des pertes. Les collisions concernent chaque dossier, avec comparaison ordinale sensible ou insensible à la casse. La longueur des noms compte les unités UTF-16 comme .NET ; les caractères de contrôle sont traités selon leur catégorie Unicode. Les pertes de contenu, noms, taille, types et métadonnées invalides bloquent toujours. Seules date de modification, commentaire et attributs peuvent être abandonnés après consentement. Le réducteur retourne une copie indépendante et ne modifie jamais le plan source.

Contrôle ciblé Debug réussi : copie indépendante des octets, chemins imbriqués, trois pertes de métadonnées, date au format aller-retour avec fraction de sept chiffres et décalage négatif, refus sans consentement, acceptation explicite, réduction, préservation des métadonnées prises en charge, contenu manquant, collision Ä/ä, comparaison sensible à la casse, longueur UTF-16 d’un caractère supplémentaire, contrôle DEL, politique de noms, nom de volume, métadonnées invalides, taille excessive et erreurs de paramètres.

Compilation Debug de MediaFileSystems réussie. Les quatre erreurs ont été générées avec Argos dans les 29 langues, relues et corrigées (notamment négations japonaises/coréennes, textes arabes restés anglais et chinois traditionnel). Les 30 catalogues DLL ont été compilés. Audit : 1366 fichiers C++, 56 clés App, 29 langues, conforme. Le contrôle temporaire et ses sorties ont été supprimés après utilisation.


## Injection et destinations concrètes

Le service `FileSystemMigrationService` est maintenant porté avec les sept destinations du C# : Amiga OFS/FFS, FAT12, Apple DOS, ProDOS, Apple III SOS et Commodore DOS. Le catalogue réutilise les politiques de noms et limites des écrivains existants. Les attributs Apple DOS et Commodore ne sont conservés que lors d’une migration entre volumes du même système de fichiers, conformément au C#.

`AppleSos::write` réutilise ProDOS, ajoute le marqueur SOS à l’offset défini par les constantes de reconnaissance déjà présentes, et conserve le profil et les adresses fournis. Aucun second écrivain ProDOS ni nouvelle DLL n’est créé.

Avant l’injection : validation du plan et consentement aux pertes, puis vérification de la présence de tous les secteurs, de leur taille et de leur initialisation à zéro. Après l’écriture : vérification de la géométrie, du nombre de secteurs, de l’unicité des indices et de chaque adresse. Capacité déclarée, nombre logique et mode d’adressage d’origine sont conservés. Les dimensions dépassant les limites représentables sont refusées avant allocation. L’annulation est propagée aux parcours et écrivains ; toutes les allocations sont possédées par des objets RAII.

Compilation Debug MediaFileSystems réussie. Contrôle ciblé réussi sur les sept destinations : injection d’un fichier puis relecture de son contenu et de l’identifiant du système de fichiers, capacité déclarée distincte conservée, adresses inchangées, refus de cibles non vierges ou incomplètes, annulation, système non pris en charge. FAT12 vérifié avec un commentaire : refus sans consentement, réussite avec consentement ; une adresse cible altérée est détectée après écriture. La validation conserve les attributs natifs Apple/Commodore uniquement pour le même système source. Les données initiales du contrôle ont été corrigées pour indiquer des types natifs valides (binaire Apple, PRG Commodore), sans changer le comportement porté.

Les cinq erreurs d’injection ont été générées avec Argos, relues et corrigées dans les catalogues concernés, puis les 30 catalogues DLL ont été compilés. Audit final : 1371 fichiers C++, 56 clés App, 29 langues, conforme. Le contrôle temporaire et ses sorties ont été supprimés.


Le service global de migration de MediaEngine (choix du format/conteneur et écriture du fichier de sortie) reste à raccorder.
