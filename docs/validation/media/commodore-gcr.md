# Validation GCR Commodore 1541/1571 — 15 novembre 2026

36 fichiers C# originaux sont compilés sans modification de leur dépôt : format, codec/checksum, descriptions, décodeur, encodeur, modèles, primitives de flux et socle d’encodage. Toutes leurs empreintes correspondent à l’inventaire. Aucun algorithme de référence ni contrat utilisé n’est remplacé par le banc.

536 cas comprennent 60 calculs de piste avec valeurs int32 extrêmes, encodage de pistes de 1, 3 et 21 secteurs, cylindres/frontières de zones, deux faces, attributs de piste explicites et de casse différente, identifiants disque extrêmes, `tracksPerSide` étendu/négatif/débordant, limites communes et propres au codec. Les cas invalides comparent la catégorie d’exception et le nom du paramètre concerné.

Les sorties d’encodage comparent nombre/SHA-256 de toutes les cellules, index et nombre/SHA-256 des intervalles de flux. Toutes les pistes valides sont ensuite décodées par les deux implémentations. Les 380 fixtures de décodage comprennent les 256 marques possibles, longueurs de sync autour du seuil, en-têtes/données isolés ou multiples, enregistrements tronqués aux frontières de symbole/octet, checksums incorrects, flips de bits, pistes/numéros extrêmes, gaps, coupes de piste, différentes durées de cellule et intervalles atypiques/nuls.

Le résultat complet compare identité, confiance/durée estimée (tolérance relative 1e-10 pour les flottants), nombre/hash des octets décodés, toutes les structures avec type/position/longueur et description anglaise exacte, tous les secteurs avec coordonnées, taille/code, intégrité nullable, position/type de contrôle et contenu nullable. Les autres métadonnées restent absentes. La limite de confiance est exercée par les pistes de 21 secteurs.

L’annulation initiale de l’encodeur et du décodeur est vérifiée. Les six clés des 29 langues sont chargées depuis les DLL et comparées aux catalogues relus, soit 174 textes. Un contrôle supplémentaire vérifie le maintien de FormatData dans le catalogue coréen Commodore 900.

- Compilation complète/déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **28 994 contrôles, aucune différence**, 512 cas valides, 24 refusés, 72 pistes encodées et 380 fixtures décodées.
- Audit des littéraux : **355 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `ecc9fe95c51038667348f447aee8634983a4dc1a21873f6c60bd45f41049a768`.

Le banc temporaire `build/verification-commodore-gcr/`, ses références/fixtures/exécutables et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La reconstruction SCP 1541/1571 et les conversions restent distinctes et non validées par ce bloc.
