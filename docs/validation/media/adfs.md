# Validation ADFS — 15 novembre 2026

Les builds de production et les bancs **Debug et Release passent** : chacun rapporte **11 352 contrôles, zéro différence**, avec **3 693 cas de référence**, **68 images reconnues** et **261 résolutions de traductions**.

## Référence C# et couverture

Le banc compile les sources originales ADFS, FileCore, modèles d'entrées/volumes et horodatages en liens de lecture seule. Des adaptateurs fournissent les blocs mémoire, les petits contrats de documents nécessaires et les interfaces ; les algorithmes de parcours, formatage et adaptation de volumes par blocs proviennent des sources C#. Les douze empreintes des sources ADFS/date ont été contrôlées contre l'inventaire.

- **90 images** : anciennes cartes BBC S/M/L et D, nouvelles cartes E/F, fragments séparés d'un même fichier, sous-répertoires, copie primaire corrompue avec reprise de la secondaire, deux copies invalides, signatures et séquences incohérentes, secteurs absents ou courts, adresses/longueurs invalides, fichiers vides, noms vides, attributs, ordre de ponctuation, capacité maximale des répertoires, cycles, références répétées, profondeurs 64/65 et accès sur la seconde face L.
- **3 584 couples load/execute** : centisecondes, transitions de calendrier, limites des champs, marqueur présent/absent, date et description technique exacte.
- **12 cas d'en-tête** : BBC aux tailles prises en charge et hors limites, format D ; longueurs et SHA-256 des octets comparés au C#.
- **7 sélections de volumes par blocs** : début non aligné dans le parent, volume minimal, longueur non multiple de 256, plage hors parent et sélection erronée. La version C# utilise son fichier `.Blocks.cs` original.

Les volumes sont comparés sur leur nom, identifiants, capacité/espace libre, nombre d'avertissements et chaque entrée récursive : nom, ordre, nature, taille, date, commentaire technique, attributs, référence de stockage, validité, contenu absent/vide et SHA-256 du contenu. Les listes profondes sont sérialisées avec une limite adaptée au banc ; le premier essai avait rencontré la limite JSON par défaut, corrigée dans le banc sans modifier le lecteur.

## Fiabilité complémentaire

Les contrôles C++ supplémentaires vérifient les drapeaux d'intégrité invalides, la conservation des octets accompagnée d'un diagnostic, l'espace libre devenu inconnu et l'annulation des sondes/lectures. Un échec injecté de la source par blocs est traité comme un secteur indisponible et produit une erreur ADFS codée lors de la lecture du volume.

Pour le caractère paresseux de la vue par blocs, le banc mesure la taille de chaque requête, limitée à un secteur de 256 octets, et les adresses effectivement lues, bornées au volume sélectionné. Le cumul d'octets lus n'est pas utilisé comme preuve de chargement intégral : des secteurs peuvent être relus pendant détection et exploration.

Les neuf clés de diagnostic dans les 29 langues ont été générées avec Argos, relues et corrigées lorsque nécessaire (anglais résiduel, sens incorrect, paramètres et chinois traditionnel). Les 261 textes sont chargés depuis les DLL de langues installées et comparés aux catalogues corrigés. L'audit des littéraux passe sur 252 fichiers C++ ; `git diff --check` passe.

SHA-256 du manifeste C# final : `cb5913ed79372feb35715fca787906645d89972b19efb6ef974bde1cda543b4b`.

Le banc, ses images synthétiques, références, fichiers compilés et artefacts sont regroupés dans `build/verification-adfs/`, supprimé après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La validation couvre le système de fichiers et ses en-têtes, pas les conteneurs ADF Acorn de MediaEngine ni la façade applicative, encore à porter.
