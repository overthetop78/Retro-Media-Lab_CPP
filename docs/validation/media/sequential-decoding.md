# Validation du registre séquentiel et de l’adaptateur MFS

16 novembre 2026 : compilation Debug MSVC x64 réussie après correction d’un include manquant de RecognitionContext (build/sequential-decoding-build.log). Le contrôle natif temporaire passe (build/sequential-decoding-smoke.log). Il vérifie propriété des données/métadonnées/positions, validation des modèles et confiance NaN/infinie/hors bornes, registre nul, compatibilité 0/1/plusieurs avec machine explicite, sélection présence de blocs puis confiance puis taille, égalités stables et absence de candidat.

Les diagnostics, arguments des segments non décodés et nom interne sont contrôlés. Un adaptateur moteur est réellement fourni à MediaExplorer::createDefault : le volume séquentiel est détecté, les fichiers MFS sont construits et les vues d’entrées analysées sont produites. Le test utilise des décodeurs de contrôle fournis au registre, pas un protocole machine inventé. Volume incompatible, annulation et durée de vie document/décodeur à travers une future différée sont aussi vérifiés par weak_ptr.

Sept messages générés par Argos dans les 29 langues puis relus/corrigés : décodeur, confiance, volume, sélection, non-décodé, négations et arguments notamment. Les 30 DLL de catalogues compilent (build/sequential-decoding-catalogs.log). Audit PASS sur 1214 fichiers C++, 29 langues (build/sequential-decoding-audit.log).

Le projet et les binaires temporaires sont supprimés après consignation. Aucun média utilisateur et aucun ajout au workflow GitHub. La composition par défaut du moteur, les protocoles réels et le raccordement App restent à réaliser ; cette validation n’en déclare pas l’achèvement.
