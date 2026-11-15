# Décodage ISO MFM et lecture binaire

## Référence et découpage

Le bloc 24 complète le codec ISO MFM à partir de 37 fichiers C# originaux inchangés. [Validation](../../../validation/media/iso-mfm-decoding.md).

`Models/Flux/BitReader` fournit les recherches de motifs et lectures d’octets bruts, MFM et FM32. `Codecs/Iso/MfmRead` collecte les marques et lit les champs/CRC ; `MfmDecoder` produit les secteurs et structures ; `MfmPll` sélectionne la reconstruction temporelle. Les contrats de piste, CRC et tailles communs sont réutilisés. Aucun matériel, fichier ou GUI n’intervient dans le décodage.

## Lecture bornée

Les plages sont contrôlées par soustraction avant accès. Les motifs variables acceptent 1..32 bits. MFM consomme seize cellules par octet, FM32 trente-deux ; seules les cellules de données sont interprétées, conformément à la source. Une séquence MFM incomplète retourne l’absence. Un nombre nul retourne une séquence vide même avec un offset invalide, comme en C#.

Le C++ refuse avant allocation les nombres négatifs ou supérieurs à la limite représentable en cellules int32 avec le diagnostic structuré `BitReadCount`. Le C# levait notamment OverflowException pour un nombre négatif. L’annulation est vérifiée avant et pendant les opérations longues.

## Marques, champs et circularité

Le décodeur reconnaît trois mots 4489 suivis d’un octet MFM. FE porte CHRN et CRC ; FB/F8 identifient données normales/supprimées. Le CRC couvre A1 A1 A1, la marque et les données concernées. Un code N supérieur à sept conserve un secteur de taille zéro, sans tentative de lecture du payload.

La recherche des marques commence uniquement dans le flux original, avec une queue circulaire copiée limitée à 20 000 cellules pour les champs qui chevauchent la jonction. Les marques de données du début sont aussi présentées après la fin du flux pour l’appairage du dernier en-tête. Les offsets de structures de données circulaires peuvent donc dépasser la longueur originale. L’appairage prend la première marque après l’en-tête et avant le suivant ; une copie circulaire appairée empêche de réémettre la marque initiale comme non appairée.

Les en-têtes incomplets conservent les valeurs nulles/zéro source. L’intégrité globale est fausse si un CRC disponible est faux, vraie si les données sont décodées sans CRC faux, sinon absente. Les intégrités d’en-tête/données et DeletedDataMark restent séparées. Le décodeur ne remplit pas artificiellement les champs de CRC enregistrés que le C# laisse vides.

Les octets décodés suivent l’ordre stable des segments : FE + CHRN/CRC disponibles et marque de données + payload/CRC disponibles. Une marque inconnue produit une structure de synchronisation, sans ajouter son octet aux données décodées. Une marque de données non appairée ajoute uniquement sa marque. La confiance utilise tous les en-têtes et structures de données, même incomplètes.

## Sélection PLL

Les sept facteurs sont essayés dans l’ordre 1, 0,98, 1,02, 0,96, 1,04, 0,94 et 1,06. Seule la tentative centrale peut terminer immédiatement lorsque tous ses secteurs possèdent des données et une intégrité vraie. Sinon le premier score maximal est conservé : 1 000 par intégrité vraie, 10 par payload présent, 1 par secteur. Le comportement int32 du score source est conservé. Les erreurs du rappel et l’annulation sont propagées.

## Traductions et portée

Les sept clés sont rangées dans `MediaEngine/iso-mfm-decoding.json`. Génération : `python scripts/translations/translate.py --all --files MediaEngine/iso-mfm-decoding.json`, depuis en-US et avec la liste commune d’invariants. La relecture des 29 langues a corrigé les paramètres CHRN perdus, les marqueurs de protection résiduels et plusieurs libellés techniques altérés par Argos. Les champs techniques et les paramètres restent intacts.

La reconstruction sectorielle SCP 1581, le routage général, les registres et documents ne sont pas réalisés par ce bloc. Les résultats concernent les données synthétiques de référence, pas une acquisition matérielle.
