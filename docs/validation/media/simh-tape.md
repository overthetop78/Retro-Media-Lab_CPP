# Validation SIMH

16 novembre 2026 : compilation Debug MSVC x64 réussie sans avertissement observé (build/simh-tape-build.log). Le contrôle temporaire natif passe (build/simh-tape-smoke.log) : records pairs/impairs, mots encadrants et drapeau erreur, trois marqueurs, métadonnées/offsets, avertissements avec arguments, aller-retour exact de la fixture, tri par position et stabilité à positions égales. Les entrées tronquées, bits réservés, longueur nulle avec drapeau erreur, dépassement et mots incohérents sont rejetés en lecture et en reconnaissance. Le writer refuse métadonnées absentes, marqueur avec données et record après EOM. Une source lève pendant la copie : la destination préexistante reste intacte ; l’annulation du reader et du writer est aussi vérifiée.

Quatre messages générés par Argos dans les 29 langues puis relus/corrigés : mot binaire, record, remplissage, tri, fin de support, négations et arguments notamment. Les 30 DLL de catalogues compilent (build/simh-tape-catalogs.log). Audit PASS sur 1202 fichiers C++, 29 langues (build/simh-tape-audit.log).

Le projet de contrôle, données et binaires temporaires sont supprimés après consignation. Aucun média utilisateur ni ajout au workflow GitHub. La composition/exploration globale reste distincte de ce contrôle.
