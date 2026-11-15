# Validation des readers Atari

Compilation Debug de MediaEngine et dépendances réussie (`build/atari-readers-build.log`). Les casts constants des signatures ont d’abord produit C4310 ; ils sont remplacés par une extraction masquée des octets. Compilation finale sans avertissement (`build/atari-readers-final-build.log`). Les trente DLL de catalogues sont compilées, avec mise à jour ciblée des deux dernières corrections linguistiques (`build/atari-readers-catalogs.log`, `build/atari-readers-catalog-corrections.log`). Solution Visual C++ régénérée.

Un unique banc en mémoire vérifie :

- Priorité d’un BPB valide à 80 cylindres/1 tête sur le repli de même capacité à 40 cylindres/2 têtes ; géométrie conservée dans le document et intégrité logique initiale conforme au C#.
- Rejet d’un BPB à 40 secteurs par piste, au-delà de la limite ST 36, puis sélection du premier repli 9 secteurs/2 têtes.
- Readers MSA, ATR et ATX sur un petit conteneur synthétique chacun ; signatures, préfixes, image sectorielle et octets accessibles à travers MediaDocument.
- ATR invalide décliné ; format demandé incompatible décliné ; annulation et erreur d’E/S conservées comme exceptions au lieu d’un résultat false.

```
PASS: ST BPB precedence and fallback policy, ST/MSA/ATR/ATX common readers, signatures/prefixes, initial integrity and probe cancellation/IO propagation
```

Résultat conservé dans `build/atari-readers-smoke.log`. Aucun média externe, aucune comparaison C# massive et aucune ancienne suite rejouée. Le banc et ses artefacts sont supprimés après consignation.

Les trois messages sont générés avec Argos dans 29 langues et relus rapidement. Les contresens observés sur octets, multiples positifs, cylindres/têtes/pistes et chinois traditionnel sont corrigés. Le contrôle des paramètres a détecté une accolade supplémentaire arabe, corrigée lors de cette relecture.

Audits : 856 fichiers C++ pour constantes/catalogues, 29 langues ; 844 fichiers pour les frontières entre composants, zéro violation constatée. Les sept références source sont vérifiées par empreinte. Ces contrôles ne déclarent pas terminés les writers ni la composition générale de reconnaissance.
