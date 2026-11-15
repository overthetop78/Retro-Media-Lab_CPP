# Commodore DOS — construction des volumes

## Référence et organisation

Le bloc 19 reprend `CommodoreDosVolumeWriter`, `CommodoreDosVolumeBuilder`, `CommodoreDosWritableGeometry`, les politiques de nom/type et les dispositions C#. Les 19 sources originales compilées dans le banc sont inchangées. [Validation](../../../validation/media/commodore-dos-writer.md).

Tout reste dans MediaFileSystems : `WritePolicy.cpp` valide les choix et noms, `WriteGeometry.cpp` résout les géométries standard, `Allocation.cpp` réserve et attribue les blocs, `WriteFiles.cpp` écrit données/répertoires/REL, `WriteSystem.cpp` construit en-têtes/BAM et `Writer.cpp` coordonne le résultat. Le plan sectoriel possède ses octets ; aucune référence au plan de migration ni à la politique ne subsiste après l’appel. Aucun accès matériel ou GUI n’est ajouté.

## Contrat et fidélité

`CommodoreDos::write` reçoit un MigrationPlan, un identifiant de format, une WritePolicy et un jeton d’annulation. Il retourne un SectorWritePlan. Les identifiants sont résolus sans casse et normalisés vers les formats canoniques. Seules les géométries standard sont construites : D64 35 pistes, D71 deux faces de 35 pistes, D81 80 pistes logiques. Ce point diffère de la lecture, qui accepte aussi les images D64/D71 étendues.

La politique valide un type de base SEQ, PRG, USR ou REL, ajoute Closed et exige une longueur REL non nulle lorsque REL est le défaut. Les fichiers provenant déjà de Commodore DOS conservent leur octet de type et leur longueur REL extraite des attributs, sans leur ajouter Closed. Les autres métadonnées non consommées par le writer C# ne sont pas inventées.

Le volume doit avoir un nom PETSCII non blanc, de seize unités UTF-16 au maximum. Les entrées doivent être des fichiers plats avec contenu présent, sans enfant, et nom valide. Les noms dupliqués ne sont pas rejetés, comme dans le constructeur source. Un fichier vide n’obtient aucun bloc de données ; un REL vide obtient néanmoins un secteur latéral.

Les secteurs système sont réservés avant le répertoire. Le répertoire préfère les secteurs suivant l’en-tête puis utilise l’allocateur commun. L’allocation trie les blocs libres par distance de leur piste globale à la piste centrale du catalogue, puis par bloc logique. Les données utilisent 254 octets par secteur ; le lien final stocke le nombre utilisé plus un. Les REL référencent au plus six secteurs latéraux de 120 pointeurs chacun, avec tables croisées et longueur d’enregistrement. Les compteurs de blocs incluent données et secteurs latéraux.

Les en-têtes et BAM conservent l’identité disque GW et les types DOS 2A/3D du C#. Tous les secteurs, y compris ceux libres, figurent dans le plan avec leur adresse. La publication vers un fichier est une responsabilité MediaEngine distincte ; le writer ne touche pas de destination. L’annulation et les erreurs détruisent les buffers locaux par RAII.

## Diagnostics et limites

Six diagnostics structurés couvrent type, longueur REL, format, entrée invalide, disque plein et dépassement REL. Le catalogue `MediaFileSystems/commodore-dos-writer.json` est généré avec Argos depuis en-US, relu puis corrigé dans les 29 langues. Les invariants sont communs au projet.

Ce bloc ne constitue pas encore un moteur complet de migration : l’orchestration des plans, registres et documents reste à porter. La publication des plans en D64/D71 est ensuite portée dans le [bloc 20](commodore-container-writer.md). La lecture Commodore DOS est décrite [séparément](commodore-dos.md).
