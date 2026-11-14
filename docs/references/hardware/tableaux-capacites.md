# Tableaux des capacités du lecteur et du contrôleur

Sources consultées le 14 novembre 2026 ; réutiliser ces références lors des prochains ajouts.

## États et origine

Chaque ligne possède une capacité, un résultat, une source et un détail. PASS est vert, FAIL rouge, les informations et les contrôles non effectués sont neutres. La source distingue une annonce du lecteur, une annonce du pilote et un essai réellement exécuté. Un FAIL sur une annonce signifie que la fonction n’est pas annoncée ; un FAIL sur un essai décrit l’échec de cet essai. Un manque de droits n’est pas converti en capacité absente.

## Optique

Les descripteurs MMC récupérés par GET_CONFIGURATION sont conservés avec leurs profils et numéros de fonctions. Le tableau expose lecture CD/DVD/BD, C2, CD-Text, lecture aléatoire, Multi-Read, streaming, modes d’écriture annoncés et profils normalisés du lecteur. Aucun profil courant du support inséré ni contenu de celui-ci n’est présenté. L’extraction brute reste non vérifiée tant qu’un essai correspondant n’a pas été réalisé.

- [FEATURE_DATA_CD_READ](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddmmc/ns-ntddmmc-_feature_data_cd_read) : bits CD-Text et C2 ; le booléen n’est exploité que lorsque le descripteur est complet.
- [FEATURE_DATA_DVD_READ](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddmmc/ns-ntddmmc-_feature_data_dvd_read) : annonce de lecture DVD ; les champs non documentés ne sont pas interprétés arbitrairement.
- SDK Microsoft installé, `shared/ntddmmc.h`, version 10.0.28000.0 : enum FEATURE_NUMBER et FEATURE_PROFILE_TYPE utilisés pour les codes de fonctions et de profils ; les codes inconnus restent affichés sous forme numérique.

## Interfaces de stockage

[STORAGE_ADAPTER_DESCRIPTOR](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddstor/ns-ntddstor-_storage_adapter_descriptor) fournit les limites de transfert et de scatter/gather, l’alignement des buffers, le mode PIO, la file de commandes et les transferts synchrones accélérés. Les champs de version appartiennent aux métadonnées HBA annoncées par le pilote ; ils ne constituent pas une mesure de débit ou une négociation USB vérifiée. L’absence du mode PIO ne permet pas de conclure à elle seule à une capacité DMA particulière.

Le SDK `shared/ntddscsi.h` définit IOCTL_SCSI_GET_ADDRESS et SCSI_ADDRESS : port, chemin/bus, cible et LUN. L’adressage est celui exposé par le pilote, sans supposer qu’il correspond au numéro imprimé sur un connecteur physique.

Le GUI expose le nom du contrôleur, son pilote, son emplacement, son bus et ces propriétés d’interface, ainsi que le résultat des commandes et de l’accès brut. Le sélecteur du contrôleur ne concatène plus les modèles des disques raccordés ; les connexions sont numérotées. Le modèle ATA, le numéro de série, la capacité, la géométrie et le contenu du support ne sont pas affichés dans ces tableaux. Aucun SMART, TRIM ou autre attribut du disque n’est présenté comme une capacité démontrée du contrôleur.
