# Interfaces de disques

Sources Microsoft consultées le 14 novembre 2026 :

- [CM_Get_Parent](https://learn.microsoft.com/en-us/windows/win32/api/cfgmgr32/nf-cfgmgr32-cm_get_parent) : rattacher les disques au contrôleur de l'arbre PnP, sans supposer qu'une lettre de volume identifie l'adaptateur.
- [BusReportedDeviceDesc](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/devpkey-device-busreporteddevicedesc) : nom fourni par le bus pour identifier les ponts USB plutôt qu'un libellé générique de classe.
- [IOCTL_STORAGE_QUERY_PROPERTY](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddstor/ni-ntddstor-ioctl_storage_query_property) et [STORAGE_ADAPTER_DESCRIPTOR](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddstor/ns-ntddstor-_storage_adapter_descriptor) : propriétés du périphérique et du pilote de l'adaptateur, maximum de transfert, alignement et file de commandes. Ces informations ne prouvent pas la prise en charge de toutes les commandes ATA/SCSI.
- [CreateFileW](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) : accès au périphérique physique ; accès zéro pour les métadonnées, GENERIC_READ uniquement pour les secteurs d'essai. Les droits Windows peuvent interdire l'accès brut.
- [Géométrie étendue](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntdddisk/ni-ntdddisk-ioctl_disk_get_drive_geometry_ex) : capacité et secteur logique ; certaines piles amovibles peuvent nécessiter la géométrie historique. Ne pas inventer une taille physique de secteur lorsque le pilote ne la fournit pas.

L'inventaire couvre les classes HDC et SCSIAdapter, dont les contrôleurs AHCI/NVMe et les cartes PCI/PCIe exposées par leurs pilotes. Un identifiant PCI ne suffit pas à distinguer un contrôleur soudé sur la carte mère d'une carte additionnelle : garder l'identifiant, le pilote et l'emplacement réels, sans cette supposition. Aucun exemplaire de carte additionnelle n'est disponible pour la validation physique actuelle.

Les lecteurs Zip/Jaz/SyQuest sont des disques amovibles. Un lecteur présent sans capacité de média exploitable peut être identifié sans qu'une lecture du support soit validée. Aucun montage ni formatage n'est requis par les services ; l'analyse du système de fichiers reste extérieure à la gestion de l'interface.
