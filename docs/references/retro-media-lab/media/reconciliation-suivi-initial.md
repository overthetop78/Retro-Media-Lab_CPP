# Réconciliation des tâches initiales 87 à 90

17 novembre 2026. Aucun backend modifié ni nouvelle batterie exécutée pour cette réconciliation.

| Tâche | Preuves dans le code courant | Validation conservée |
|---|---|---|
| 87 : registre sectoriel | Exploration/Composition/Catalog.cpp conserve les 21 lecteurs dans l’ordre ; SectorReader.h et SectorRegistry.h exposent le contrat ; Registry.cpp, Identity.cpp et Document.cpp portent sélection, identité et adaptation. Atari/FormatRecognition.* regroupe KFile, CLK et BootDisk. | validation-interpretation-filesystems.md ; la batterie sector-registry préparée a été abandonnée. |
| 87 : catégories | ErrorKind distingue ArgumentOutOfRange et InvalidArgument ; ProDos/Codec.cpp conserve les gardes write16/write24. Les filtres Amiga Directory/Recovery retiennent InvalidData/Overflow/ArgumentOutOfRange, Coherent et BBC InvalidData, ADFS Directory InvalidData, ADFS BlockView Io/InvalidData comme prévu par leurs contrats. | Revue actuelle des points cités ; fabriques-diagnostics-consommateurs.md. |
| 88 : documents et volumes | Representations.h et MediaDocument.h ; VolumeDescriptor à membres privés et accesseurs en lecture seule ; constructeur validé dans Partitioning/Volumes.cpp ; MBR/GPT filtrent InvalidData et propagent les autres catégories. | contrats-documents-resultats.json : six comparaisons conservées et cinq contrôles natifs réussis, sans extrapoler à toute la batterie initiale. |
| 89 : ImageDisk/Epson | Epson/Geometry.*, ImageDisk/{ImageDisk.h,Constants.h,Models.cpp,Reader.cpp,Writer.cpp,Metadata.cpp,Builder.cpp}, raccordements CMake et adaptateurs. Le catalogue s’appelle image-disk.json. | reconciliation-inventaire-cpc-imd.md, readers-imd-cpc-hfe.md et epson-imagedisk-writers.md. |
| 90 : CPC DSK | CpcDsk/{CpcDsk.h,Constants.h,Internal.h,Reader.cpp,Builder.cpp,Writer.cpp} et Models/Sectors/UniformLayout.*. | reconciliation-inventaire-cpc-imd.md, readers-imd-cpc-hfe.md, adf-dfs-cpc-writers.md. |

Les anciennes batteries temporaires 87/89/90 n’ont pas été exécutées : docs/validation/media/bancs-prepares-retires.md fait foi. Leur annulation est désormais explicite dans les tâches, au lieu d’une case ouverte pouvant faire croire à un lancement encore attendu. Les corrections de chemins reflètent le rangement réel, sans créer de seconde implémentation.

La compilation Release globale et le chargement/déchargement des trois DLL ont déjà réussi ; voir validation-distribution-release.md. Cette preuve de construction ne remplace pas les preuves fonctionnelles ciblées conservées. Les relectures de traduction encore ouvertes restent visibles dans les tâches.
