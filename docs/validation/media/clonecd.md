# Validation CloneCD

Le 16 novembre 2026 : compilation Debug MSVC x64 MediaEngine réussie (`build/clonecd-build.log`), sans avertissement relevé ; 30 DLL de catalogues compilées (`build/clonecd-catalogs.log`). Les quatre erreurs sont générées par Argos dans les 29 langues, puis relues et corrigées : invariants CloneCD/IMG/SUB, négation du coréen, arabe partiellement anglais, terminologie et chinois traditionnel notamment.

Le contrôle natif temporaire réussit : `PASS CloneCD v2/v3, numeric/hex/overflow, sessions/TOC, immutable fields, 3 modes, IMG/SUB, stored pregap, indexes/flags/catalog/ISRC, missing/alignment/ranges, cancellation` (`build/clonecd-smoke.log`). Audit des constantes et catalogues : PASS 1156 fichiers C++ (`build/clonecd-audit.log`).

Les fixtures synthétiques comportent deux sessions, trois entrées TOC et trois pistes MODE1/2352, AUDIO et MODE2/2352 sur six secteurs IMG, avec SUB associé. Vérifications des octets des secteurs, fenêtres utilisateur, sous-canaux, INDEX 00/01/02, pregap stocké, flags, catalogue, ISRC, ordre des entrées et sessions affectées. Les collections possédées restent inchangées après modification d’une copie.

Contrôles négatifs : sections/propriétés en double, alias numériques de session ou piste, comptes incohérents, version/mode inconnus, signe plus interdit, négation hexadécimale débordante, index dupliqué, IMG/SUB mal dimensionnés, index inversés/hors fichier/manquants, fichier associé absent et identifiant de format incompatible. Les entiers hexadécimaux signés et les versions 2/3 sont contrôlés positivement. Les fichiers refusés peuvent être remplacés immédiatement ; aucun handle de descripteur ou de données n’est conservé. Une requête annulée échoue sans chargement supplémentaire.

Les fichiers de test et leurs CCD/IMG/SUB/projets/binaires temporaires sont supprimés après consignation. Aucun média utilisateur ni test n’est ajouté au workflow GitHub. Cette validation concerne CloneCD, et ne constitue pas une validation de l’ensemble de MediaEngine, de la composition globale ou d’App.
