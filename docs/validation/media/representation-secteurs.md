# Validation de la représentation sectorielle

Compilation Debug de MediaEngine et dépendances réussie sans avertissement (`build/sector-representation-build.log`). La modification du contrat MFS a recompilé les fournisseurs concrets présents, notamment les vues ADFS et Commodore. La solution Visual C++ est régénérée.

Un seul banc en mémoire contrôle les nouveaux raccordements :

- Un fournisseur instrumenté refuse toute E/S avant acquisition de la mécanique ; blockAddress ne lit aucun octet et conserve l’adresse explicite.
- La lecture physique utilise (7,1,10), conserve la tête enregistrée 99 et termine son accès.
- Un bloc absent et l’énumération bornée ne déclenchent pas d’accès ni d’E/S supplémentaire.
- Une erreur d’E/S laisse la mécanique inactive et conserve la catégorie Io.
- Une image éparse à adresse négative utilise le bloc logique 1 ; le mode écriture emploie BeginWrite. Le pointeur du bloc retourné reste identique à celui de l’image d’origine, avec tags et préfixe enregistré conservés.
- La représentation rejoint MediaDocument et la détection générique des volumes, sans copie ni lecture de média externe.

```
PASS: payload-free addresses, physical/logical access, shared sector metadata, idempotent binding, missing blocks, lazy enumeration and exception cleanup
```

Résultat conservé dans `build/sector-representation-smoke.log`. Aucune batterie comparative C#, aucun ancien test rejoué. Le banc et ses artefacts sont supprimés après consignation.

Audits : 843 fichiers C++ pour constantes/catalogues, 29 langues ; 831 fichiers pour les frontières entre composants, zéro violation constatée. Aucune nouvelle chaîne utilisateur ni nouvelle erreur : les validations réemploient les diagnostics traduits existants pour image nulle et dépassement de plage signée.

La granularité C++ est la requête de bloc, et non chaque indexation d’un octet du vector déjà livré. Ce choix est explicitement documenté dans la référence et l’inventaire ; aucun contrôle ne prétend établir une équivalence d’événements par octet avec l’énumérateur C#.
