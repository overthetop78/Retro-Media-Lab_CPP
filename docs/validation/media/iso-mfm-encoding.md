# Validation encodage ISO MFM — 15 novembre 2026

21 sources C# originales, y compris les contrats consommés, sont compilées sans modification de leur dépôt. Leurs empreintes correspondent à l’inventaire. Aucun algorithme ou contrat de référence n’est remplacé dans ce banc.

879 cas couvrent : une matrice CRC de 4 096 combinaisons d’octet/polynôme/initialisation, 104 séquences CRC supplémentaires de zéro à 4 096 octets comparant calcul complet, span, mise à jour et ajout ; tailles ISO reconnues et voisines, valeurs int32 extrêmes et les 256 codes de taille ; champs enregistrés, CRC fournis de zéro à trois octets, intégrités nulles/valides/invalides, données supprimées, codes enregistrés incohérents, toutes les tailles ISO et limites de champs.

Les pistes physiques couvrent 0/1/2/10 secteurs, capacités de zéro à 100 000 cellules avec frontières autour des octets et du gap initial, exigences complètes activées/désactivées/négatives, remplissage et troncature, durées nulles ou dépassant les limites. Les pistes logiques comprennent plusieurs secteurs et métadonnées incohérentes conservées comme dans le C#. Les sorties valides comparent identité, nombre/SHA-256 de toutes les cellules et nombre/SHA-256 des intervalles, ainsi que l’index. Les refus comparent la catégorie d’exception et le paramètre concerné.

Un premier passage a relevé le nom de codec abrégé ; la constante a été corrigée depuis la source. Le banc Ninja temporaire ne reprenait pas le préfixe d’inclusion MSVC français utilisé par le build du projet et conservait sa copie compilée de l’en-tête. Il a été configuré avec ce même préfixe puis reconstruit : 176 dépendances d’en-têtes sont enregistrées pour son unité de compilation. Le build principal avait déjà son probe de préfixe et des dépendances suivies ; il n’a pas été modifié pour ce problème du banc.

L’annulation initiale du calcul CRC, de l’ajout du CRC et de l’encodage est vérifiée. Les cinq clés des 29 langues sont chargées depuis les DLL et comparées aux catalogues relus, soit 145 textes.

- Compilation complète/déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 028 contrôles, aucune différence**, 560 cas valides, 319 refusés et 416 pistes encodées.
- Audit des littéraux : **369 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `686a93dbfe7d35c5f861adf3350d46878fbd455c886ac12906111e9d6452f184`.

Le banc `build/verification-iso-mfm-encoding/`, ses références/exécutables et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur ou test matériel, aucun ajout au workflow GitHub. Le décodage MFM et la reconstruction SCP 1581 ne sont pas validés par ces résultats.
