# Validation ADF — 15 novembre 2026

Les builds de production **Debug et Release réussissent**. Le banc final rapporte dans les deux configurations **4 158 contrôles, zéro différence**, **11 écritures identiques octet par octet** et **87 résolutions de traductions**.

## Source et comparaison

Le projet de référence compile les neuf sources ADF originales C#, les modèles/builders sectoriels et les writers linéaires, ainsi que les lecteurs ADFS/FileCore/FAT12 utilisés pour valider les candidats et explorer les images. Les adaptateurs fournissent les données de reconnaissance, les contrats de documents nécessaires et le transport de sortie fichier ; ils ne réécrivent pas les algorithmes de lecture ou de sélection des profils. Les trois constantes sectorielles Amiga nécessaires à ses géométries sont reprises de la définition MFM lue dans le C# ; le codec MFM n'est pas inclus ni déclaré porté. Les neuf empreintes des sources ADF sont inchangées.

Le corpus contient **99 images synthétiques** et **3 564 sondes de reconnaissance**, croisées avec extensions et identifiants demandés. Il couvre :

- Les huit profils Acorn/Amiga, leurs numérotations et la variante rembourrée ; détection automatique ou explicite DOS Plus.
- Les troncatures avant/après cartes et répertoires, le dernier secteur court, cartes/séquences invalides, tailles inconnues et capacités déclarées inférieures à la géométrie complète.
- Les DiscRecords du début et du bloc d'amorçage, les vraies cartes FileCore E/F, la copie secondaire lorsque la primaire est corrompue, et la priorité des tailles Amiga même en présence d'un en-tête Acorn.
- Les demandes de format incompatibles/inconnues et le filtre spécial `.640`, conformément aux branches C#.

Pour chaque image lisible, la comparaison inclut identifiant, dimensions, capacité, nombre logique/disponible de blocs et possibilité d'écriture. Une empreinte SHA-256 agrège **tous les secteurs** dans l'ordre logique : numéro logique, adresse CHR, longueur, marqueur d'intégrité, adresse enregistrée et octets. Des contrôles distincts refusent les métadonnées physiques inventées. Le total des contrôles compte une empreinte comme un contrôle, pas comme autant de tests indépendants que de secteurs.

L'exploration Acorn compare noms/identifiants des volumes, capacité/espace libre, avertissements, ordre et contenu des entrées, attributs, références, validité et commentaires techniques. Le raccordement au classifieur est exercé sur les entrées extraites. Les onze sorties C++ sont comparées directement aux fichiers produits par les writers C#, puis rouvertes. Les en-têtes de base BBC/D des fixtures proviennent des formatters C#.

## Fiabilité

Les lectures initiales d'images complètes sont mesurées avant exploration/énumération : elles restent inférieures à la taille complète, avec requêtes d'en-tête limitées à 4096 octets. Les tests d'écriture injectent un secteur absent puis court ; les diagnostics attendus sont vérifiés, la destination sentinelle reste identique et aucun temporaire orphelin n'apparaît. L'annulation en cours d'écriture préserve également cette destination. Sont aussi contrôlés l'annulation de lecture, le format d'écriture inconnu, la taille invalide et ses arguments, l'exploration Acorn refusée pour Amiga et le refus d'une somme de contrôle vide.

Les 87 textes (trois clés dans 29 langues) sont générés avec Argos puis relus/corrigés, notamment la traduction incorrecte de writer en auteur, les phrases restées en anglais et le chinois traditionnel. Ils sont chargés depuis les DLL installées et comparés aux catalogues relus. L'audit des littéraux passe sur 260 fichiers C++ ; `git diff --check` passe.

SHA-256 du manifeste de résultats C# : `5bc4c6d9e1f25341d7ab3209d2b7f6e6bb82f94fa07baed54f4aeb0d74175c9a`.

Le banc, ses images, sorties, références et fichiers compilés sont regroupés sous `build/verification-adf/`, supprimé après validation. Aucun média utilisateur, test matériel ni ajout aux workflows GitHub. Les documents/enregistrements applicatifs, conversions et systèmes de fichiers Amiga restent hors de ce bloc et à porter.
