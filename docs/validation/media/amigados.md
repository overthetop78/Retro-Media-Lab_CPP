# Validation AmigaDOS — 15 novembre 2026

Les vingt sources C# originales ont été compilées sans modification dans un exécutable de référence ; les vingt empreintes correspondent à l’inventaire. Les fixtures sont synthétiques et ne contiennent aucun média utilisateur.

## Comparaison

844 cas : 238 scénarios d’images, 293 dates, 266 noms/politiques, 19 types d’entrée, 18 sommes/motifs indisponibles et 10 variantes. Les scénarios utilisent des images de référence et des modifications ponctuelles, secteurs absents ou courts, sans multiplier les copies de médias.

Les 236 volumes lisibles sont comparés en profondeur : ordre/noms/types, taille, dates, commentaires, attributs, références de stockage, intégrité de métadonnées/données, SHA-256 du contenu, enfants, capacité, espace libre et nombre d’avertissements. Les deux autres scénarios contrôlent les erreurs attendues. Les racines signées/protégées, variantes, chaînes corrompues/cycliques, extensions, fichiers tronqués et récupération sont couverts.

Quatre créations OFS/FFS × DD/HD sont identiques au C# octet par octet, puis relues. Elles comportent des sous-répertoires, fichiers vides, un fichier nécessitant une extension, noms Latin-1 et collisions de casse. Les deux champs de date de la racine C# sont normalisés à un instant fixé et la somme de racine recalculée : les appels `UtcNow` de la source ne permettent pas une comparaison déterministe autrement. Les dates des entrées sont déjà imposées. Cette normalisation ne concerne aucun autre champ.

Des contrôles supplémentaires vérifient intégrité enregistrée, annulation en lecture/création, noms invalides, variante de création refusée, géométrie trop grande/trop petite, volume plein, contenu manquant, bitmap court et création/relecture au 31 décembre 9999. Une première version du décodage de date a échoué sur cette dernière année à cause d’un dépassement dans une conversion de durée ; le décodage utilise maintenant le calendrier civil et tous les cas passent.

L’exploration ADF → AmigaDOS → analyse des entrées est vérifiée. Les 23 messages des 29 langues, soit 667 textes, sont chargés depuis les DLL installées et comparés aux catalogues relus.

## Résultats

- Compilation complète et déploiement Debug et Release réussis avec MSVC x64/Qt 6.12.
- Dans chaque configuration : **3 219 contrôles, zéro différence, 236 volumes et quatre créations comparées**.
- Audit des littéraux : 269 fichiers C++, 56 clés d’erreur App, 29 langues ; catalogues médias également vérifiés. `git diff --check` passe.
- SHA-256 du manifeste de résultats C# : `00263403734fb40de8d17e216ad1bd8574938694ea119a233287bdbedef0586f`.

Le banc et ses fichiers compilés/références sont regroupés dans `build/verification-amigados/`, supprimé après validation. Aucun test matériel, modification de l’interface ou ajout aux workflows GitHub. Ces résultats valident ce bloc, pas le portage complet des bibliothèques médias.
