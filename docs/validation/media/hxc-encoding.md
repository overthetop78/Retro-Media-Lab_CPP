# Validation ciblée du codage HxC Stream

Le 16 novembre 2026, compilation Debug MediaEngine réussie avec MSVC 19.44 et LZ4/zlib déjà embarquées, sans avertissement dans `build/hxc-encoding-build.log`. La compilation incrémentale finale et les 30 DLL Constants/langues réussissent (`build/hxc-encoding-final-build.log`, `build/hxc-encoding-catalogs.log`).

Un programme natif unique vérifie :

- l'empreinte exacte `EB0D8DFFBB0403420748C49ECD367A0EF91998CF1298E68B152D0EC95FE3C350`, calculée indépendamment avec Python struct/hashlib pour deux révolutions aux résolutions 25 et 20 ns, ainsi que sa variation avec la protection en écriture ;
- le CRC `D202D277` de `123456789` avec la graine du C#, calculé indépendamment avec Python zlib ;
- les clés de métadonnées par piste, les en-têtes, longueurs, alignements et CRC des chunks ;
- la décompression des blocs produits avec LZ4_decompress_safe, les métadonnées filtrées et le choix de résolution 40/20 ns ;
- les impulsions attendues [50,100,250,50,100,250], les échantillons d'index 0/25/50 et le signal de protection ;
- les quatre tailles du code d'impulsion, y compris les valeurs 128, 16384 et 2097152 ;
- les rejets sans révolutions, des temps non représentables exactement, d'une impulsion nulle, de transitions après l'index, d'une capture E/S trop longue et l'annulation.

Résultat : `PASS HxC digest/CRC/LZ4 chunks/exact resolutions/variable pulses/sampled indexes/metadata/error bounds`, dans `build/hxc-encoding-smoke.log`. Ce contrôle n'utilise pas de média utilisateur et ne réexécute aucune ancienne suite de codecs. Il ne vérifie pas encore le lecteur HxC ni la publication atomique multipiste.

Les catalogues anglais et les 28 traductions Argos ont été relus et corrigés, notamment révolution physique, impulsion, index, taille du bloc et négation de la conservation exacte. Les invariants HxC, HxC Stream et LZ4 sont protégés. L'audit statique passe sur 1007 sources C++ et 29 langues (`build/hxc-encoding-audit.log`).

Le programme de vérification, ses sources, son projet CMake et ses binaires temporaires sont supprimés après consignation ; les journaux restent dans build, ignoré de Git.
