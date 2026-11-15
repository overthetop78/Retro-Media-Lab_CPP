# Validation de la fabrique et des adaptateurs

Compilation Debug de MediaEngine et dépendances réussie sans avertissement (`build/document-adapters-build.log`). Les trente DLL de catalogues sont compilées (`build/document-adapters-catalogs.log`) et la solution Visual C++ est régénérée.

Un seul banc synthétique, sans média externe ni comparaison C#, vérifie les nouveaux raccordements : représentation flux avec surfaces 0/1, trois révolutions et six transitions ; fabrique secteur/flux sans copie de l’image et avec métadonnées ; normalisation/déduplication des capacités ; exécution différée après destruction de l’adaptateur et du document appelants ; transmission des métadonnées au writer flux ; rejet d’une liste de fichiers produits vide avec InvalidData.

Les trois readers Commodore reconnaissent leurs longueurs et produisent les documents sectoriels attendus. Les cartes d’erreurs des D64/D71 sont conservées. L’adaptateur DOS refuse le couple format D64/extension D71 et réécrit une petite image synthétique D64 avec sa carte d’erreurs complète, octet pour octet. L’adaptateur D81 valide la géométrie attendue. Un cas 36 pistes est refusé par la validation géométrique du writer DOS avant sortie.

```
PASS: flux surfaces/counters, document factories, deferred writer ownership, output validation, D64/D71/D81 common readers and complete DOS error-map output
```

Le résultat est conservé dans `build/document-adapters-smoke.log`. La relance accompagne seulement l’ajout du contrôle de la validation 35/40 pistes. Aucune ancienne suite n’est rejouée. Le banc et son fichier temporaire sont supprimés après consignation.

Les onze nouveaux messages sont générés avec Argos dans 29 langues et relus rapidement ; les contresens observés sur écrivain/composant, compteurs, plage d’entiers signés, callbacks et extensions sont corrigés. Le chinois traditionnel est corrigé après la génération restée en caractères simplifiés. Cette relecture ne revendique pas une certification linguistique professionnelle.

Audits : 851 fichiers C++ pour constantes/catalogues, 29 langues ; 839 fichiers pour les frontières entre composants, zéro violation constatée. Les références de neuf sources sont vérifiées par empreinte ; MediaWritingComposition reste explicitement partielle dans l’inventaire.
