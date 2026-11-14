# Capacités et entrées matérielles

Sources consultées le 14 novembre 2026.

- [GET_CONFIGURATION](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddcdrm/ni-ntddcdrm-ioctl_cdrom_get_configuration) : interrogation des profils et fonctions annoncés par un lecteur optique. Ce résultat ne prouve pas la fidélité d'une extraction réelle.
- [CD Read](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddmmc/ns-ntddmmc-_feature_data_cd_read) : indicateurs CD-Text et pointeurs d'erreurs C2. Ne pas en déduire un support complet des sous-canaux ou GD-ROM.
- [En-tête de configuration](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddmmc/ns-ntddmmc-_get_configuration_header) : longueurs et codes en big-endian ; contrôler les limites avant de lire les descripteurs.
- [Types des entrées audio](https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/ne-mmdeviceapi-endpointformfactor) : LineLevel et Microphone sont distincts ; conserver les types inconnus plutôt que les exclure.
- [États audio](https://learn.microsoft.com/en-us/windows/win32/coreaudio/device-state-xxx-constants) : les entrées désactivées ou dont la prise est débranchée ne font pas partie de DEVICE_STATE_ACTIVE. Les présenter sans les déclarer disponibles pour l'acquisition.
- [Interfaces matérielles SetupAPI](https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetdeviceinterfacedetailw) : utiliser le chemin d'interface retourné pour CreateFile, et non le nom interne du nœud PDO. Le tampon GET_CONFIGURATION reste sous la limite de longueur MMC.
- [LTO](https://www.ibm.com/docs/en/ts4500-tape-library?topic=media-overview-lto-tape-drive) : exemple de bandes de données dédiées, distinctes des disques amovibles Zip/Jaz/SyQuest.

Le C# actuel est la référence pour les préférences existantes ; ces interrogations optiques et types d'entrées sont une extension demandée, pas un changement de moteur d'acquisition. L'accès aux HDD vise le périphérique physique, pas un système de fichiers monté par Windows. Le test réel de l'adaptateur/disque attend son branchement et son identification par l'utilisateur.

Architecture C# vérifiée : `RetroMediaLab.Infrastructure.csproj`, `Infrastructure/Hardware/` et `Infrastructure/HostTools/`. La DLL C++ `Infrastructure.dll` reprend les services et modèles hors GUI, sans lien vers App ni Qt Widgets. Le nom reste celui du C# tant qu'aucun autre nom n'est retenu. Les vues App commandent les services et présentent leurs résultats ; elles ne possèdent pas les ressources de détection ou les threads de test.
