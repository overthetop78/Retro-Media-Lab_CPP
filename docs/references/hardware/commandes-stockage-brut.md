# Accès brut et identification des interfaces

Sources consultées le 14 novembre 2026, à réutiliser pour la suite du portage.

## Pont USB HDD branché

Le parent PnP USB VID 152D / PID 2338 correspond à l’entrée JMicron JM20337/8 du [catalogue officiel smartmontools](https://github.com/smartmontools/smartmontools/blob/main/drivedb/drivedb.h), qui sélectionne usbjmicron. Le [code officiel du transport](https://github.com/smartmontools/smartmontools/blob/main/lib/scsiata.cpp) décrit une commande constructeur 0xDF : lecture du registre 0x720F pour sélectionner le port, puis transfert entrant de la réponse ATA IDENTIFY (0xEC). Le registre est uniquement lu ; aucun port ambigu n’est choisi arbitrairement. Le service RML construit seulement ces requêtes d’identification, sans reprendre l’implémentation GPL ni intégrer smartmontools comme dépendance.

## Windows

[IOCTL_SCSI_PASS_THROUGH](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddscsi/ni-ntddscsi-ioctl_scsi_pass_through) permet les commandes via le périphérique de classe. Microsoft impose une requête synchrone et un handle disposant des droits lecture/écriture. Ces droits d’ouverture ne signifient pas que RML écrit : les CDB du service sont exclusivement DATA_IN, sans formatage, changement de configuration ni commande d’écriture. Le handle et les données sont libérés par RAII, même en cas d’exception. Le service tourne hors du thread GUI, avec un délai de commande de dix secondes.

La file de commandes annoncée par un pilote est distincte du passage des commandes SCSI/ATA. Une identification réussie prouve cette requête sur ce chemin, pas toutes les commandes possibles. Un échec ne prouve pas l’impossibilité de lire un système de fichiers étranger.

## Supports et système de fichiers

HDD et lecteurs amovibles Zip/Jaz/SyQuest : lire les secteurs du périphérique physique permet de récupérer des données sans interprétation du système de fichiers par l’hôte. Infrastructure gère l’interface et les capacités ; le décodage des données reviendra aux bibliothèques média prévues. La prise en charge du support physique dépend du lecteur et de son interface.

Optique : Microsoft fournit [IOCTL_CDROM_RAW_READ](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddcdrm/ni-ntddcdrm-ioctl_cdrom_raw_read) et [RAW_READ_INFO](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddcdrm/ns-ntddcdrm-__raw_read_info), qui adressent les secteurs et le mode de piste XA/CDDA, sans chemin de fichier. Le passage SCSI est une autre voie de commande. La lecture brute optique ne suppose donc pas un système de fichiers reconnu par Windows. Cela ne rend pas un lecteur CD compatible DVD/BD/GD-ROM : les capacités physiques et les commandes acceptées doivent être vérifiées séparément. Le code courant interroge les capacités MMC ; une extraction complète et ses modes optiques restent à porter, sans prétendre qu’ils sont déjà implémentés ni validés sur tous ces formats.

Greaseweazle acquiert des transitions de flux de disquettes ; les interfaces HDD/Zip/optiques ordinaires exposent des secteurs. L’objectif commun est de ne pas dépendre du système de fichiers de l’hôte, sans promettre le même niveau physique d’acquisition.
