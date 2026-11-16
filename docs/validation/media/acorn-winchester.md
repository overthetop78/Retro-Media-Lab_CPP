# Validation ciblée Acorn Winchester

Le 16 novembre 2026 : compilation Debug MediaEngine réussie avec MSVC 19.44 (`build/acorn-winchester-build.log`). Une ambiguïté de nom avec `std::filesystem::exists` et la conversion explicite de la taille de signature ont été corrigées pendant la compilation du port. La compilation finale ne signale pas d'avertissement. Les 30 DLL Constants/langues sont construites (`build/acorn-winchester-catalogs.log`).

Un programme natif temporaire a vérifié :

- code DSC nul conservé pour 256 octets, géométries 32/33 et 17/18 secteurs, support 9 secteurs de 1024 octets ;
- descripteur inchangé conservé intégralement, mise à jour de géométrie sans perte des octets annexes ;
- deux suites attendues de neuf octets pour les paramètres physiques Adaptec, et rejet des tailles/paramètres invalides ;
- création atomique DAT/DSC avec préallocation, retour des chemins et conservation du DSC natif ; refus du remplacement ;
- lecture complète, fichiers associés et reconstruction des données ; image partielle avec plages indisponibles et message localisable ;
- refus d'un DAT trop long pour le DSC ou non aligné sur sa taille de bloc ;
- reconnaissance Hugo, choix explicite et refus d'un autre format ;
- refus du base64 invalide, création d'un DSC neuf et annulation sans changer la destination existante.

Résultat : `PASS Acorn Winchester geometry/DSC preservation/Adaptec parameters/DAT DSC pair/partial ranges/Hugo/create/preallocation/cancellation`, dans `build/acorn-winchester-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs n'ont été employés. Le programme, ses données synthétiques et binaires sont supprimés après consignation.

Les neuf erreurs et deux messages sont générés par Argos dans les 29 langues, relus et corrigés, notamment base64 invalide, géométrie inconnue et secteurs absents. Audit statique réussi sur 1052 sources C++ et 29 langues (`build/acorn-winchester-audit.log`). Ces vérifications ne valident pas encore le codec de pistes Adaptec, Linux/macOS ou l'intégration depuis App.
