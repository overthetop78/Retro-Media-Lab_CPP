# Validation des modèles optiques

Compilations Debug de MediaEngine et dépendances réussies (`build/optical-models-build.log`, `build/optical-models-final-build.log`) sans avertissement. Les trente DLL de catalogues sont compilées (`build/optical-models-catalogs.log`). La solution Visual C++ est régénérée.

Un unique petit banc en mémoire vérifie une piste à trois secteurs avec prégap stocké et sous-canaux espacés : fenêtre utile, secteur stocké, prégap dans une destination fournie, refus du prégap dans l’overload allouant, sous-canaux, propriété des sources pendant une lecture différée, ordre des index et positions signées, sessions, listes absentes/vides, liaison mécanique idempotente, adresses logiques 99/101/102, conservation des métadonnées et fichiers associés, intégration au document et rejet des descriptions sans données lisibles. Le stride invalide et le nombre négatif de secteurs d’une ancienne description sont rejetés dans les catégories attendues.

```
PASS: optical windows, stored pregap, strided subchannels, deferred ownership, sessions, absent/empty collections, mechanical addresses and metadata-only rejection
```

Aucun média externe ni comparaison massive C#. Le banc et ses artefacts sont retirés après conservation du résultat dans `build/optical-models-smoke.log`.

Le catalogue est généré avec Argos pour 29 langues. Lecture rapide effectuée ; corrections des contresens observés, notamment piste/index, valeurs numériques positives/négatives, prégap, métadonnées et sous-canaux, ainsi que du chinois traditionnel resté en écriture simplifiée après génération. Cette relecture ne constitue pas une certification linguistique professionnelle.

Audits : 834 fichiers C++ pour constantes/catalogues, 29 langues ; 822 fichiers pour les frontières de composants, zéro violation constatée. Aucun contrôle de ce bloc ne prétend valider les conteneurs optiques encore non portés.
