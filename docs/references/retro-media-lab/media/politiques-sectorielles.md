# Politiques de reconstruction sectorielle — 15 novembre 2026

Le C# utilise ISectorImageVisualizationPolicy, sa classe de base, neuf implémentations de politiques et SectorImageVisualizationPolicyRegistry. Le même mécanisme participe à la visualisation et à l’encodage physique. Le port le range donc dans Reconstruction/Sector, séparément des contrôles App et des accès matériels.

Policy.h définit les opérations et les éléments sectoriels possédés. Policy.cpp conserve les métadonnées du C# : numéro enregistré prioritaire, taille ISO déduite de la charge utile, marque supprimée, tags indexés, adresse enregistrée, code N enregistré, intégrité d’en-tête, intégrité DATA avec repli sur l’intégrité globale et CRC enregistré. Les octets de préfixe DATA ne sont pas ajoutés à TrackSector, conformément à la politique C#.

Les comportements spécialisés restent séparés :

- Apple : découpage des blocs ProDOS/SOS de 512 octets en deux secteurs de 256 octets dans les conditions exactes du C#, conversion Lisa linéaire, choix des codes de format et cadences Macintosh/Lisa zonées.
- Commodore : regroupement 1581 par numéro de bloc, tri et limitation à 512 octets, élimination des groupes incomplets, conversion des adresses D81 et cadences GCR zonées 1541/900.
- RX02 : découpage des blocs de 512 octets en deux secteurs de 256 octets ; les blocs courts sont ignorés.
- Atari : conversion du modèle linéaire à une seule unité par cylindre lorsque sa géométrie le demande.
- JFD : correspondance de format sensible à la casse, présence de blocs, densité identique et tags ordinaires ; ni géométrie spéciale ni densité ne sont devinées.
- Profils rares : réutilisation du catalogue du bloc 61.
- Correspondances exactes/par préfixe : copie des paramètres et validation des identifiants obligatoires.

Composition/SectorPolicies.cpp construit les onze politiques dans l’ordre original. Registry retourne le premier résultat compatible et conserve la propriété des politiques ; une politique retournée reste vivante après destruction du registre. Aucun cache global n’est ajouté.

Les comparaisons des identifiants de catalogue ASCII conservent le comportement du bloc 61. Les correspondances personnalisées utilisent une comparaison ordinale Unicode sans normalisation ni expansions de casse. Le contrôle évite notamment l’assimilation ASCII/Unicode. L’identité U+FEFF est conservée dans les constructions d’images et dans le constructeur ISO, avec deux contrôles natifs et deux cas C# spécifiques.

Gardes C++ explicites : un Item ne peut contenir un bloc nul ; les champs transférés vers les adresses d’encodage 32 bits doivent être représentables. Une densité JFD absente ou nulle produit un diagnostic traduit, au lieu d’un accès optionnel invalide ou d’une division par zéro. Les erreurs d’arguments des politiques et de cylindre Lisa sont également traduites dans les 29 langues.

Le bloc ne porte pas encore SectorImageTrackEncoder, l’orchestrateur qui regroupe et encode toutes les pistes d’une image ; il sera raccordé à ces politiques et au registre d’encodage existant.
