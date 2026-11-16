# Formatage sectoriel des pistes runtime

Références C# en lecture seule : `SectorTrackFunctions.cs` et méthode `FormatTrack` de `MediaRuntimeDisk.Tracks.cs`.

Les fonctions internes de validation, remplacement et projection réutilisent les représentations sectorielles existantes. La validation vérifie la piste physique et les champs fournis. Le remplacement à géométrie fixe exige les mêmes numéros uniques, adresses enregistrées et tailles, sans marque supprimée ni erreur de CRC d’en-tête ou de données que ce mode ne peut représenter. Chaque champ garde son indice logique initial ; l’ordre physique vient de la nouvelle liste.

Le remplacement flexible, utilisé pour CPC DSK, accepte les numéros répétés et renumérote les indices logiques après tri stable par adresse physique et numéro. L’ordre physique d’origine reste enregistré séparément. Il refuse les positions logiques manquantes dont seuls les descripteurs du conteneur connaissent la signification. La projection interne conserve les anciens indices dans l’ordre physique puis alloue les champs supplémentaires ; elle ne prétend pas autoriser leur sauvegarde dans un conteneur qui les perdrait.

Le formatage runtime ImageDisk passe par ses descripteurs existants : conservation du commentaire et des autres pistes, mode FM/MFM et débit explicites ou repris de l’ancienne piste, numéros répétés, adresses enregistrées et états de données supprimées/CRC. Les champs et tailles non représentables sont refusés avant de remplacer le document. Les changements sectoriels en attente sont inclus ; le nouvel état n’est publié qu’après construction réussie de toute la représentation.

## Vérification ciblée

Compilation Debug réussie. Le banc vérifie les indices logiques et l’ordre physique inversés, la conservation des données des autres pistes, les adresses enregistrées, préfixes, diagnostics et codes d’intégrité dans les représentations. Refus des doublons en mode fixe, des tailles incohérentes, des erreurs d’intégrité non représentables, des pistes hors limites et des positions manquantes en mode flexible ; l’image source reste intacte.

Une image ImageDisk temporaire garde une position indisponible sur une autre piste après formatage et sauvegarde. Deux secteurs de même numéro, une marque de données supprimées, une erreur de données et le mode FM 300 sont relus dans le fichier publié. Une image CPC DSK étendue conserve trois champs dans l’ordre fourni, dont deux numéros répétés et une adresse enregistrée différente. Une IBM 720 vérifie la conservation des indices logiques et des autres pistes après inversion de l’ordre physique puis sauvegarde et relecture.

L’audit des constantes passe sur 1 413 fichiers C++ et 29 langues. Aucun diagnostic supplémentaire ni nouvelle traduction ne sont nécessaires : les refus utilisent les diagnostics runtime existants. Le banc, ses exécutables et ses images temporaires sont supprimés après utilisation.

La lecture de piste physique, les fenêtres d’écriture, la nouvelle tentative de projection différée et les pistes Winchester restent dans la suite du portage. Le formatage sectoriel ne prétend pas remplacer ces fonctions.
