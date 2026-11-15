# Images brutes IBM/MSX — référence et réorganisation

Source C# en lecture seule : `RetroMediaLab.MediaEngine/Images/Formats/Floppy/Raw/` (IbmPcGeometry/Catalog, détecteurs IBM, readers/writers IBM et MSX, catalogues MSX, LinearSectorImageWriter et exceptions), `Images/Reading/Recognition/Msx/MsxBootSectorProbe.cs`, `Interfaces/IAtomicImageFileWriter.cs` et `Images/Writing/AtomicFileWriter.cs`.

## Géométries et lecture

Le catalogue IBM conserve ses onze profils, dont IBM 1680 et DMF explicitement distincts. La recherche par capacité exclut DMF comme le C#. Un BPB valide pour la longueur exacte est prioritaire ; sinon les capacités du catalogue sont utilisées. L’identifiant d’une géométrie issue du BPB dépend de sa capacité, mais sa géométrie réelle n’est pas remplacée par la géométrie canonique. Le détecteur depuis un boot seul ne demande pas la longueur complète et dispose du repli sur descripteur FAT historique.

Les quatre profils MSX sont conservés. À 368 640 octets, le descripteur F8 choisit 1DD (80 cylindres, une tête) ; les autres descripteurs choisissent 2D (40 cylindres, deux têtes). Le probe OEM MSX reste insensible à la casse et valide le BPB sans exiger que son compte de secteurs égale la longueur externe, comme le C#. La combinaison longueur/descripteur est ensuite validée séparément.

Les identifiants partagés ont une seule définition dans MediaFileSystems/FormatIds.h ; le catalogue FAT12 et les profils Engine la réutilisent. Les constantes et associations des profils Engine restent dans Formats/Raw/Constants.h. Les modèles sectoriels et l’écriture linéaire commune sont dans Models/Sectors ; l’écriture de fichier atomique reste dans Sources. Aucune dépendance aux contrôles App ni algorithme MediaAnalysis direct n’est ajouté au moteur.

Les lecteurs lisent uniquement le premier secteur au moment de l’ouverture. Les autres secteurs sont lus sur demande par la source propriétaire déjà validée. Le résultat est identique en données/adresses au découpage C#, sans ses copies complètes intermédiaires. Une absence de checksum dans le conteneur brut reste explicitement une intégrité inconnue ; le C# utilisait implicitement true via la valeur par défaut du bloc. Aucune validité physique n’est inventée.

## Écriture et publication

Les writers ne réparent ni ne réécrivent le BPB. Ils contrôlent géométrie et profil puis publient les secteurs dans l’ordre logique, sans remplissage implicite des secteurs absents ou tronqués. IBM scan conserve la géométrie source, et le profil DMF reste sélectionnable à l’écriture. Les quatre profils MSX réutilisent la même primitive linéaire.

La primitive commune permet le contrôle du format d’origine ou son changement explicitement autorisé après validation de la géométrie. Les dimensions multipliées sont bornées avant comparaison. Comme correction de fiabilité, la capacité d’écriture MSX vérifie aussi la présence et la taille des secteurs ; la condition C# ne vérifiait que la géométrie.

La destination n’est publiée qu’après succès complet et vérification finale de l’annulation. Les parents sont créés comme dans le C#. QSaveFile est employé sans repli par écriture directe ; sa destruction abandonne le temporaire non validé. Les erreurs d’ouverture, écriture et publication sont distinctes. Référence officielle déjà consultée et conservée dans la [note QSaveFile](../../tooling/preferences-et-cadre-natif.md), sans nouvelle recherche identique.

## Diagnostics

Les erreurs de conteneurs/fichiers appartiennent au domaine MediaEngine, distinct des diagnostics de systèmes de fichiers. Douze clés et paramètres, générés avec Argos dans les 29 langues puis relus/corrigés. Le diagnostic d’annulation de stockage déjà traduit est réutilisé afin d’éviter le contresens « médias » au sens de presse. Les invariants MSX et paramètres sont contrôlés. L’audit vérifie maintenant les clés, doublons, paramètres et traductions en-US de tous les diagnostics MediaEngine/MediaFileSystems.

## Restant

Ces fonctions sont internes à MediaEngine. L’enregistrement complet dans la reconnaissance, les contextes/documents, les demandes d’interprétation et la façade publique App restent à porter. Les fichiers readers C# restent donc partiels dans l’inventaire. Ce bloc ne se substitue pas aux autres formats bruts, aux flux, aux conteneurs structurés et aux autres systèmes de fichiers.
