# Validation WAVE séquentiel

16 novembre 2026 : compilation Debug MSVC x64 réussie sans avertissement observé (build/wave-tape-build.log). Le contrôle temporaire passe (build/wave-tape-smoke.log) : PCM 8/16/24/32 bits avec 1, 2 et 32 canaux à 768000 Hz ; reproduction exacte du fichier ; données impaires/remplissage RIFF ; plusieurs chunks data concaténés ; chunk inconnu vide conservé et writer refusant sa perte ; fichier vide à 1 Hz ; avertissement de fin ; bornes RIFF, tag PCM, alignement et canaux invalides ; refus de range coupant une trame ; annulation avec destination intacte.

Trois messages générés avec Argos dans les 29 langues, relus/corrigés : PCM entier, unités, négation, arabe non traduit, mélanges anglais/espagnol, invariants WAVE et chinois traditionnel notamment. Les 30 DLL de catalogues compilent (build/wave-tape-catalogs.log). Audit PASS sur 1192 fichiers C++, 29 langues (build/wave-tape-audit.log).

Le projet et les médias/binaires temporaires sont supprimés après consignation. Aucun média utilisateur ni ajout au workflow GitHub. Ce contrôle ne prouve pas l’intégration globale encore à porter.
