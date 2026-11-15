# Validation de l’analyse du flux brut — 15 novembre 2026

Le banc compile quinze sources C# originales sans remplacer d’algorithme : RawFluxDecoder, ses définitions, l’estimateur temporel, descriptions et modèles. Leurs empreintes SHA-256 correspondent à l’inventaire.

Les 2 779 cas comprennent 1 445 combinaisons des aides et 1 334 analyses complètes. Les aides croisent intervalles nuls/usuels/uint32 maximal, durées positives/négatives, zéro positif/négatif, NaN et infinis, valeurs minuscules/géantes et indices négatifs/zéro/positifs. Elles vérifient classification, priorité des anomalies, bornage et arrondis. Les analyses comprennent toutes les 1 296 séquences de quatre intervalles parmi six valeurs, huit captures limites et trente captures déterministes de 1 024 intervalles.

Les comparaisons portent sur identifiant, nom source, confiance, cellule estimée, absence d’octets/secteurs et chaque structure ordonnée : type, position, longueur, description exacte résolue en anglais. Les tests n’allouent pas de capture géante pour provoquer un dépassement d’offset int32 ; cette accumulation est exprimée explicitement en arithmétique définie dans l’implémentation.

Dix-neuf contrôles supplémentaires vérifient annulation initiale, sélection du flux brut sur une révolution vide et clé de nom localisé, puis les quatre classes de codecs réels. Les encodeurs synthétiques ISO FM/MFM et Commodore GCR/900 fournissent des secteurs connus ; le registre explicite et la sélection automatique doivent retrouver leur contenu et intégrité, et partager les résultats. Deux captures SCP réelles synthétiques vérifient aussi collecte et cache ISO avec les cinq décodeurs enregistrés. Aucun média utilisateur n’est utilisé. Les 87 textes des DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **2 885 contrôles, aucun écart**, 2 779 cas et 87 textes.
- Audit : **417 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `a4bc980ba62dfef5cd683b36d30a3e56ec3f6e8e07160be5e78590b32f912339`.

Le banc build/verification-raw-flux, ses captures, binaires et journaux de comparaison sont supprimés après vérification. Aucun test ajouté au workflow GitHub. Le catalogue par défaut complet n’est pas déclaré porté par ce bloc.
