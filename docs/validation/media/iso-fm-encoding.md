# Validation encodage ISO FM — 15 novembre 2026

23 sources C# originales sont compilées sans modifier leur dépôt ; leurs empreintes correspondent à l’inventaire. Aucun algorithme ni contrat de piste n’est réécrit pour la référence.

653 cas couvrent les 256 codes de taille (toutes les tailles ISO pour les codes 0..7, payload vide pour les codes supérieurs), tailles reconnues/voisines, codes fournis incohérents, codes enregistrés prioritaires avec payloads incohérents, valeurs limites et int32 extrêmes des champs, face/cylindre physiques, les combinaisons d’intégrité d’en-tête/données/deleted et CRC fournis de zéro à trois octets. Les pistes physiques/logiques comprennent zéro/un/trois/dix secteurs, attribut d’activation exact ou autre valeur, exigence complète nulle/positive/négative, capacités autour des marques/champs/gaps et troncatures. Les durées nulles, dépassements int32 de capacité ou uint32 d’intervalle et les pistes vides sont aussi comparés.

Les sorties valides comparent identité et nom affiché, nombre/SHA-256 de toutes les cellules, nombre/SHA-256 des intervalles uint32 dans l’ordre, et durée d’index. Les refus comparent le type d’exception et le nom du paramètre lorsque le C# en fournit un. L’annulation initiale est vérifiée. Les deux textes des 29 langues sont chargés depuis les DLL et comparés aux catalogues relus : 58 contrôles. Le banc Ninja utilise le préfixe d’inclusion MSVC français du projet.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **712 contrôles, aucune différence**, 653 cas dont 526 valides et 127 refusés.
- Audit : **383 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `f033ba1b83d02568a4681fa0386f08b89a2c7bd40f9cbe5402c25bf916938cdf`.

Le banc `build/verification-iso-fm-encoding/`, ses références/exécutables et journaux de comparaison sont supprimés après vérification. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. Ce bloc ne valide pas le décodeur FM ni les reconstructeurs ISO.
