# Interfaces de disques : validation Windows

14 novembre 2026. Toutes les interrogations sont effectuées par Infrastructure.dll ; App présente les choix et les résultats. Aucun formatage, écriture de secteur, montage ou changement de partition n'est effectué.

## Identification

Deux contrôleurs AHCI SATA et deux contrôleurs NVMe présents sont identifiés par leurs nœuds PnP PCI/PCIe, pilotes et emplacements. Les classes exposent également les contrôleurs virtuels Windows. Les ponts USB sont ajoutés depuis les parents réels des disques et leur nom fourni par le bus : notamment « USB to ATA/ATAPI bridge » et « USB Zip 100 ».

Le GUI sépare interface et disque raccordé. Ajouter une configuration enregistre l'interface avec son identifiant stable, sans utiliser la lettre O:/P: comme identité. Le disque choisi pour le test doit appartenir à cette interface ; la sélection et les rapports survivent au changement de langue et à la reconstruction de la présentation.

## Capacités déclarées par les pilotes

| Interface | Transfert maximal annoncé | File de commandes annoncée |
| --- | --- | --- |
| AHCI SATA, deux contrôleurs | 131072 octets | Oui |
| NVMe, deux contrôleurs | 262144 octets | Oui |
| Pont USB–SATA testé | 65536 octets | Non |
| Zip 100 USB testé | 65536 octets | Non |

Les interrogations des contrôleurs SATA/NVMe portent seulement sur leurs métadonnées via un disque rattaché, sans lecture de secteurs utilisateur. Les pilotes indiquent 512 octets logiques et 4096 octets physiques pour les disques rattachés concernés. Les deux périphériques USB ne fournissent pas de taille physique de secteur exploitable : la valeur reste inconnue, sans supposer 512 octets physiques.

## Tests réels, en lecture seule

Les volumes O: et P: ont servi uniquement à identifier les deux périphériques demandés. Les tests utilisent ensuite leur interface physique et l'identifiant PnP exact, sans parcourir de fichiers.

- Pont USB–SATA avec ST350063 0AS : capacité annoncée 500107862016 octets, secteurs logiques de 512 octets. Après autorisation de l'utilisateur et confirmation UAC, trois secteurs lus avec succès au début, au milieu et à la fin de la capacité annoncée. Aucune écriture. L'accès brut sans élévation renvoie un refus de droits, distinct d'un défaut de lecteur.
- Zip USB initialement vide : interface identifiée, aucune capacité de support validée.
- Après insertion de la cartouche par l'utilisateur : capacité annoncée 100663296 octets, secteurs logiques de 512 octets. Avec confirmation UAC, lectures du secteur initial et du secteur médian réussies ; la lecture du LBA 196607 échoue. La répétition limitée aux mêmes positions confirme le code Windows 483, ERROR_DEVICE_HARDWARE_ERROR. Cette réponse ne permet pas d'attribuer le défaut à la cartouche, au lecteur ou à son pilote. Cet incident reste un diagnostic sur cet essai. Les lectures réussies vérifient bien la capacité d’accès brut du lecteur ; elles ne doivent pas être présentées comme une absence de capacité du Zip.

La [référence des codes Windows](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes) explique leur origine ; la définition précise de 483 est vérifiée dans `WinError.h` du SDK Windows installé, ligne 3596, version 10.0.28000.0. Les erreurs affichées sont traduites ; le code Windows et le LBA restent des données techniques. Aucun essai de réparation ni lecture exhaustive n'est lancé.

Ces tests ne prouvent pas une compatibilité universelle des adaptateurs ni la disponibilité de toutes les commandes ATA/SCSI. Aucune carte PCI/PCIe additionnelle n'étant disponible, son traitement repose sur la même énumération des classes de contrôleurs ; aucune validation physique de carte additionnelle n'est revendiquée.

## Contrôles de logiciel

Le banc vérifie le rattachement des deux appareils USB à leur interface, la configuration de l'interface plutôt que du disque, les doublons, la mise à jour de disponibilité dans le service, les chemins d'outils gérés par le service, la sauvegarde, le changement de langue, la reconstruction de la présentation et le maintien des résultats. App ne contient plus de recherche récursive d'outils, de création du dossier des journaux ni de code natif de périphérique. Infrastructure ne dépend ni d'App ni de Qt Widgets.

Les sources du banc, exécutables, scripts UAC, captures, catalogues intermédiaires et rapports JSON temporaires sont supprimés à la fin de la validation. Aucun média de test ni test ajouté au workflow GitHub.

## Vérification du passage des commandes

Le pont JMicron USB 152D:2338 accepte SCSI INQUIRY puis le transport constructeur ATA IDENTIFY ; la réponse ATA contient le modèle ST3500630AS attendu. Erreur native 0, statut SCSI 0. La propriété de file de commandes reste « non » : elle ne mesure pas cette capacité de passage ATA.

Le Zip USB 059B:0033 lit correctement un secteur brut au début du média dans le dernier test limité. La requête d’identification SCSI n’est pas validée sur cette pile ; l’essai SAT renvoie un statut CHECK CONDITION. Aucune conclusion générale d’absence de commandes n’en est tirée. Aucun défaut du support n’est recherché.

Le bouton présente désormais les propriétés de l’interface, l’accès brut vérifié et les commandes d’identification réellement vérifiées. La capacité du support et les erreurs par LBA ne sont plus le résultat principal du test. Le test courant lit au maximum un secteur, sans sauvegarder son contenu, pour vérifier l’accès brut. Les essais précédents de trois secteurs restent consignés ci-dessus comme historique.

Les nouvelles chaînes ont été générées avec Argos pour les 29 langues puis relues ; corrections des traductions de « secteur » et des libellés techniques ambiguës. L’exécution Argos nécessite ici l’accès autorisé à sa configuration et ses modèles installés hors du bac à sable ; aucune réinstallation.

## Validation finale

Compilations Debug et Release réussies. Release déployée dans `build/Release/Retro Media Lab/`. Debug vérifiée par une installation distincte afin de respecter la fenêtre que l’utilisateur souhaitait garder ouverte. Le déploiement Qt inspecte désormais App.dll et Infrastructure.dll ; Qt Network et ses composants sont pris en compte.

Le banc GUI à l’échelle 150 % vérifie les deux interfaces USB, le routage vers le périphérique choisi, l’absence de doublon, la disponibilité, les chemins d’outils, la sauvegarde, le changement de langue et la reconstruction du thème. Le contrôle visuel final confirme que les rapports restent lisibles dans la zone défilante, sans comprimer les cartes.

Imports DLL vérifiés : Infrastructure dépend de Qt Core/Network et Translations, sans App, Qt Widgets ou Qt Gui. Audit des littéraux : 115 fichiers C++, 55 clés générales d’erreurs, 29 langues. Les clés et paramètres des 29 catalogues matériels sont cohérents. `git diff --check` passe. Aucune modification du dépôt C# et aucun commit dans ce bloc.

La validation du parcours intégré du bouton et de la demande UAC est détaillée dans `test-interface-integre.md`. Elle confirme désormais l’identification SCSI du Zip, avec un état séparé pour le rejet ATA.
