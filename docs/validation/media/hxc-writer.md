# Validation ciblée du writer HxC

Le 16 novembre 2026, compilation Debug MediaEngine réussie avec MSVC 19.44 (`build/hxc-writer-build.log`). Les 30 DLL Constants/langues sont construites (`build/hxc-writer-catalogs.log`). Les neuf nouveaux diagnostics sont générés avec Argos dans les 29 langues, relus et corrigés, notamment les négations, données base64 invalides et sauvegardes de récupération. L'audit passe sur 1020 sources C++ et 29 langues (`build/hxc-writer-audit.log`).

Un seul programme natif ciblé a vérifié :

- publication de deux sorties et réservation de 1 Mio pour un résultat final de trois octets ;
- refus du jeu vide, des chemins dupliqués sans distinction de casse et du remplacement interdit ;
- conservation des anciennes sorties après échec d'un producteur ;
- restauration de la première sortie quand la publication suivante échoue sur un répertoire existant, sans déplacer celui-ci ;
- annulation et absence de temporaires/sauvegardes résiduels ;
- ordre des pistes, conservation native octet pour octet, recodage, protection en écriture et validation du nom physique ;
- refus des données natives base64 invalides lorsque l'empreinte correspond, puis recodage lorsque l'empreinte change.

Résultat : `PASS HxC writer/native preservation/re-encoding/naming/atomic set rollback/cancellation/preallocation`, dans `build/hxc-writer-smoke.log`. Aucun média utilisateur et aucune ancienne suite de codecs n'ont été utilisés. La persistance après coupure, les erreurs physiques empêchant une restauration et Linux/macOS ne sont pas validés par ce contrôle.

Le programme temporaire, ses données synthétiques, son projet et ses binaires sont supprimés après consignation. La composition globale et le raccordement à App restent en cours.
