# Validation des représentations par blocs

15 novembre 2026.

Compilation Debug finale de MediaEngine/MediaFileSystems réussie, sans erreur ni avertissement. Le banc consommateur de la DLL compile également sans avertissement d'import/export (`build/block-representation-build.log`, `build/block-representation-smoke-build.log`).

Un petit contrôle en mémoire vérifie les plages stockées/zéro/non allouées/indisponibles, le découpage par piste CHS, les identités de liaison mécanique, le partage asynchrone après destruction de la représentation appelante, les lectures partielles de plage et la fusion de plages. Il vérifie EndAccess après erreur d'E/S et après échec partiel de BeginRead. La vue publique RepresentationInfo et son accès depuis l'interface MFS sont également contrôlés.

```
PASS: mixed mapped bytes, CHS access boundaries, unavailable-data classification, async ownership, range merge and cleanup after read/begin failures
```

Aucune batterie de comparaison C# ni média externe. Le contrôle a été relancé seulement après les modifications de liaison/interface et l'extraction de la source instrumentée générique ; les suites de formats déjà validées n'ont pas été rejouées.

LNK1163 a désigné la table virtuelle abstraite de RepresentationInfo. Aucun ILK résiduel n'était présent. La compilation réussit après application conditionnelle MSVC de novtable à cette interface pure, suivant la documentation Microsoft référencée dans `docs/references/retro-media-lab/media/representations-blocs.md`. Cela constate l'efficacité de cette correction sur ce projet sans prétendre établir une cause générale de LNK1163.

Audit des constantes/catalogues : 804 fichiers C++, 29 langues, aucun littéral applicatif hors constantes. Audit architectural : 792 fichiers, aucune violation. Les validations des blocs utilisent les erreurs typées du contrat MFS : corruption, arguments, E/S et annulation restent distinguables sans dépendance MFS vers Engine.

Le catalogue block-representation a été généré par Argos dans les 29 langues et compilé avec constants dans les 30 DLL de catalogues. Le français a été relu/corrigé ; la relecture complète des autres langues reste ouverte.

Le service concret de simulation mécanique, les liaisons secteur/optique, le document reconnu global, ses façades et les formats encore marqués non portés dans l'inventaire ne sont pas déclarés terminés par ce bloc.
