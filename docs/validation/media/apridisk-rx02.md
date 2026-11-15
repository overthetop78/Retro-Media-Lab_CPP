# Validation Apridisk / RX02

- Compilation Debug : `build/apridisk-rx02-build.log`, réussie ; avertissement de masquage dans ImgReader corrigé, puis recompilation réussie sans avertissement dans `build/apridisk-rx02-final-build.log`.
- Un programme natif ciblé sur données synthétiques en mémoire : `build/apridisk-rx02-smoke.log`, PASS. Apridisk : 630 secteurs compressés, enregistrement non sectoriel ignoré, doublon remplaçant le secteur initial, réécriture non compressée puis relecture, refus d’extension incompatible et dépassement d’expansion RLE.
- RX02 : points d’adressage indépendants pour la première piste, seconde moitié et dernière piste revenant au début physique ; lecture puis reconstruction d’un dump aux octets identiques ; refus de capacité incomplète. Ces points évitent qu’une simple inversion symétrique masque une erreur d’entrelacement.
- Aucun média utilisateur ni ancienne batterie de codecs utilisés. Les données restent en mémoire ; le banc et ses binaires temporaires sont supprimés après consignation.
- Les messages sont générés avec Argos dans les 29 langues, relus et corrigés avant compilation des catalogues. Compilation des 30 catalogues réussie (`build/apridisk-rx02-catalogs.log`). Audit des littéraux : 916 fichiers, 29 langues, PASS. Audit des frontières : 904 fichiers, aucune violation. Ces contrôles statiques ne certifient pas la totalité du portage.
