# Validation ciblée CHD optique

Le 16 novembre 2026, MediaEngine compile en Debug sans avertissement observé (`build/chd-optical-build.log`, puis `build/chd-optical-final-build.log`). Le contrôle natif temporaire réussit : `PASS CHD optical CD/DVD/15-types/pregap/indexes/subchannels/alignment/V3-V4-V5-DEFLATE/cross-hunk/cache-recovery/rejections/cancellation` (`build/chd-optical-smoke.log`).

Les fixtures synthétiques vérifient les 15 alias de types de piste, les fenêtres utilisateur Mode1/Mode2 brutes, les métadonnées CHTR/CHT2, les pregaps stockés, index 0/1, postgaps, sous-canaux RW_RAW, tri des numéros et alignement du stockage à quatre frames. Le DVD v5 brut relit des secteurs stockés et zéro ; les DVD v3/v4/v5 compressés DEFLATE relisent les deux hunks. Une lecture traverse leur frontière. Un cas est également relu depuis un vrai fichier temporaire.

Une source instrumentée confirme que le hunk courant n’est pas relu inutilement. Après l’échec CRC d’un autre hunk, la lecture du premier recharge ses données : l’index du cache ne désigne pas le buffer corrompu. Les bornes des sources, la couverture des cartes, les hunks physiques hors fichier et l’annulation sont contrôlés.

Les rejets couvrent format explicitement incompatible, types inconnus, sous-canaux inconnus, doublons de clés, doublons de pistes, ambiguïté CD/DVD, couverture de frames et cycle de métadonnées. Le registre sélectionne le lecteur. Ces contrôles portent sur ce raccordement optique ; les codecs communs ne sont pas soumis à une nouvelle batterie de tests.

Les quatre diagnostics sont générés avec Argos dans les 29 langues puis relus et corrigés, notamment métadonnées, numérotation, pistes, sous-canaux et négations. Les 30 DLL Constants/langues compilent (`build/chd-optical-catalogs.log`). Audit des littéraux réussi sur 1133 fichiers (`build/chd-optical-audit.log`). Programme, fixtures et binaires temporaires sont supprimés après consignation. Aucun média utilisateur n’est utilisé. La composition globale et le fonctionnement depuis App restent à terminer.
