# Validation ciblée Adaptec

Le 16 novembre 2026 : compilation Debug MediaEngine réussie avec MSVC 19.44, sans avertissement observé (`build/adaptec-build.log`). Les 30 DLL Constants/langues sont construites (`build/adaptec-catalogs.log`).

Un programme natif temporaire a vérifié :

- contrôle d'erreur de 123456789 égal à FFE64CEB, obtenu indépendamment par parcours bit à bit en Python ; résidu nul des identités produites ;
- encodage/décodage de 33 secteurs avec paramètres, géométrie relue et données correctes ;
- fenêtre DATA appliquée à une révolution synthétique : nouveau contenu valide et secteur voisin inchangé ;
- contrôle DATA endommagé conservé avec taille connue, non utilisé pour deviner une taille inconnue ; identité ID endommagée et refus de son écriture ;
- secteur absent transformé en gap, sans identité fictive ; refus d'une identité dupliquée à l'écriture ;
- trois pistes avec un secteur déplacé vers la suivante : capacité logique de 98 secteurs, recherche du secteur déplacé, identités physiques et un seul cache de piste ;
- compteur d'accès prouvant la réutilisation du décodage sur la même piste ; lecture de blocs chevauchant deux secteurs ;
- bornes d'adresse et de lecture, révolution insuffisante et annulation.

Résultat : `PASS Adaptec error check/MFM fields/recorded integrity/geometry/DATA write window/missing sectors/relocation/cache/block reads`, dans `build/adaptec-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs n'ont été utilisés. Programme, projet et binaires temporaires supprimés après consignation.

Les treize diagnostics sont générés par Argos dans les 29 langues, relus et corrigés, notamment la rotation physique, les négations et l'identité permettant l'écriture. Audit statique réussi sur 1062 sources C++ et 29 langues (`build/adaptec-audit.log`). Ce contrôle ne prouve pas le fonctionnement du conteneur MfmEmu, l'intégration App ou l'émulation d'un contrôleur physique.
