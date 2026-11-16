# Validation des images optiques Wii U et Xbox

Le 16 novembre 2026 : MediaEngine compile en Debug MSVC x64 (`build/console-optical-build.log`). Les 30 DLL de catalogues compilent (`build/console-optical-catalogs.log`). Sept diagnostics sont générés via Argos dans les 29 langues et relus/corrigés, notamment alignement aux secteurs, références manquantes, négations, volume de stockage confondu avec volume sonore, arabe anglais et chinois traditionnel. Audit : PASS 1171 fichiers C++ (`build/console-optical-audit.log`).

Le contrôle ciblé temporaire réussit : `PASS WUD/WUX dedup/index/padding/partial chunk/logical reads/roundtrip/profile/cancellation and Xbox descriptor/root/track/atomic sector copy` (`build/console-optical-smoke.log`).

Wii U : source logique de 100352 octets, quatre blocs logiques dont les deux premiers identiques, trois blocs stockés. Vérifications des index 0/0/1/2, taille finale 131072, padding et complétion zéro, longueur logique, données après décompression et copie WUD octet par octet, lecture traversant une frontière. Une taille de bloc non puissance de deux (300) est contrôlée avec réutilisation du même bloc stocké sur sept index. Les mauvais magics, tailles de bloc, longueurs, tableaux tronqués, index hors des données, mauvais type d’extension et couverture Unavailable sont refusés. La source rejette les dépassements et tableaux non conformes. Une annulation déclenchée après une lecture pendant l’écriture WUX préserve le fichier cible existant.

Xbox : image synthétique de 36 secteurs, double magic et racine au secteur 33. Vérifications des métadonnées, volume, identifiant MFS `microsoft-xdvdfs`, piste 2048 et copie ISO exacte. Une racine vide 0/0 est acceptée ; signatures erronées, racine non alignée ou hors image, incohérence racine/taille, extension de reconnaissance et format demandé incompatibles sont refusés. Une écriture annulée préserve une cible existante.

Les projets, binaires et images temporaires WUD/WUX/ISO sont supprimés après consignation. Aucun média utilisateur ou test n’est ajouté au workflow GitHub. Le contrôle n’est pas une validation de la composition complète des services ou du fonctionnement depuis App.
