# Validation ciblée VDI

Le 16 novembre 2026 : compilation Debug MediaEngine réussie avec MSVC 19.44 (`build/vdi-build.log`), puis compilation incrémentale finale (`build/vdi-final-build.log`). Les 30 DLL Constants/langues sont construites (`build/vdi-catalogs.log`).

Un programme natif temporaire a vérifié les écritures et lectures VDI fixes/dynamiques sur trois blocs logiques : données, zéros et dernier bloc partiel. Les tailles physiques, nombres de blocs alloués, entrées de table, données reconstruites et géométrie sont contrôlés. Il vérifie un UUID aux octets indépendamment imposés, les plages non allouées, l'espace supplémentaire par bloc et les zéros du bloc dynamique.

Les refus couvrent en-tête trop court, signature, version, type, secteur, décalages, géométrie, nombre alloué, table, parent différentiel, entrée dupliquée et données hors fichier. La reconnaissance refuse les en-têtes invalides. L'annulation après écriture temporaire conserve les anciens octets de la destination. Le classement d'une erreur KryoFlux comme rejet est également contrôlé.

Résultat : `PASS VDI fixed/dynamic/sparse blocks/UUID/geometry/partial blocks/header and map validation/atomic cancellation`, dans `build/vdi-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs ne sont employés. Une erreur d'espace de noms dans le programme temporaire a été corrigée avant son exécution ; aucun défaut d'exécution VDI n'a été observé dans ce contrôle.

Les quinze diagnostics sont générés par Argos dans les 29 langues puis relus/corrigés, notamment positionnement, tables, décalages, parent différentiel et négations. VDI est ajouté aux invariants communs. L'audit passe sur 1033 sources C++ et 29 langues (`build/vdi-audit.log`). Les programmes temporaires, données synthétiques et binaires sont supprimés après consignation.

Cette validation ne prouve pas l'ouverture depuis App ni le fonctionnement Linux/macOS. Les autres conteneurs de disques durs restent en cours de portage.
