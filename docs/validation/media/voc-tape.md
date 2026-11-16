# Validation Creative Voice

16 novembre 2026 : compilation Debug MSVC x64 réussie sans avertissement observé (build/voc-tape-build.log). Le contrôle natif temporaire passe (build/voc-tape-smoke.log). Il vérifie son, continuation, silence hérité, marqueur, texte, PCM attendu, constante/fréquence, version/checksum de sortie et aller-retour des échantillons ; les entrées PCM 8/16/24/32 bits stéréo donnent le premier canal mono attendu. Une sortie de 0x1000002 échantillons vérifie la longueur 24 bits maximale du premier bloc, la continuation de cinq octets et le terminateur. Offset/bloc tronqué, compression non prise en charge, constante nulle, changement de constante, type inconnu et absence de samples sont refusés. Terminateur manquant et annulation avec destination préservée sont couverts.

Quatre messages générés par Argos dans les 29 langues puis relus/corrigés : sans signe, constance/non-zéro, terminateur, négations et invariants VOC/Creative Voice notamment. Les 30 DLL de catalogues compilent (build/voc-tape-catalogs.log). Audit PASS sur 1196 fichiers C++, 29 langues (build/voc-tape-audit.log).

Le projet de contrôle, binaires et données temporaires sont supprimés après consignation. Aucun média utilisateur intégré ni test ajouté au workflow GitHub. La composition globale reste hors de cette validation de composant.
