# Validation ciblée GDI

Le 16 novembre 2026, MediaEngine compile en Debug sans avertissement observé (`build/gdi-build.log`, `build/gdi-final-build.log`). Le contrôle natif réussit : `PASS GDI descriptor/UTF8-UTF16/Unicode/offsets/shared-tracks/raw-audio/roundtrip/atomic-cancellation/overwrite/invalid-output-rejection` (`build/gdi-smoke.log`).

Un jeu synthétique de trois pistes couvre Mode1 brut, Audio et Mode1/2048, un offset non nul et un nom contenant espace, pourcentage et accent. La lecture vérifie fenêtres utilisateur, LBA, longueurs, noms et métadonnées. Le writer produit des fichiers par piste et un descripteur UTF-8 sans BOM ; leur relecture conserve données et disposition. Un autre descripteur partage un fichier entre deux pistes avec offsets distincts et confirme le partage de source.

Les tests refusent comptes et syntaxe invalides, numéros dupliqués, LBA non croissants, nombres avec signe, tailles/contrôles non pris en charge, guillemets incomplets, fichiers absents et sortie hors du dossier. Les données mal alignées et insuffisantes sont refusées à la construction des pistes. UTF-8 avec BOM et UTF-16 avec BOM sont reconnus ; le registre sélectionne le lecteur.

L’annulation avant écriture et pendant la copie conserve les quatre fichiers existants et ne laisse pas de temporaire. Le writer peut utiliser son jeu de sortie comme source puis l’écraser atomiquement, sans altérer ses octets. Les dispositions à trou ou chevauchement, que le writer C# aurait acceptées sans garantir une relecture fidèle, sont refusées ; le pregap stocké l’est également.

Les six erreurs sont générées avec Argos dans les 29 langues puis relues et corrigées, notamment LBA, syntaxe à six champs, contrôle, secteurs, chemins et négations. Les 30 DLL de catalogues compilent (`build/gdi-catalogs.log`). Audit réussi sur 1138 fichiers (`build/gdi-audit.log`). Programme, descripteurs, pistes et binaires temporaires sont supprimés après consignation ; aucun média utilisateur ni ancienne suite n’est utilisé. La composition globale et l’intégration App restent à faire.
