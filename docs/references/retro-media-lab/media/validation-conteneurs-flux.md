# Conversion directe SCP/HFE

Référence en lecture seule : `Images/Conversion/Flux/FluxContainerConversionService.cs` et ses trois validateurs de parité C#.

`FluxConversion.h` expose l'éligibilité et la conversion asynchrone sans construction d'une image sectorielle. La copie SCP/SCP ou HFE/HFE conserve les octets du fichier source et relit la destination. Le SCP copié doit avoir un checksum valide ; tous les champs d'en-tête hors checksum, adresses de pistes, révolutions, index, comptes déclarés et intervalles sont comparés. Le HFE compare révision, géométrie, encodage, débit, cellules et timing ; le C++ contrôle aussi son indicateur de protection en écriture.

HFE/SCP crée une révolution uniforme par piste avec les constantes SCP du C#. Le parcours inverse utilise le convertisseur exact HFE existant et son PGCD des timings. Il écrit explicitement un HFE v1 : aucun repli implicite vers HFE v3 ni quantification PLL. Les révolutions multiples, remplissages incompatibles, pistes manquantes et timings non représentables sont refusés. La relecture de parité fait partie de la fonctionnalité livrée, pas seulement du test temporaire.

Les copies utilisent l'écriture atomique existante ; les fichiers de lecture sont possédés par RAII. Annulation et erreurs se propagent. Le diagnostic supplémentaire `MediaEngine.FluxContainerParity` est généré par Argos puis relu/corrigé dans les 29 langues ; les autres refus utilisent les diagnostics HFE/SCP existants.

Validation : compilation Debug du bloc assemblé réussie ; construction des 30 catalogues réussie ; audit des constantes réussi sur 1 500 fichiers C++. Un seul programme temporaire a créé une petite piste synthétique et vérifié copies identiques, HFE/SCP/HFE, détection d'un intervalle modifié, refus de deux révolutions et annulation sans sortie. Tous les contrôles ont réussi. Programme, répertoire de médias et journal temporaire supprimés. Aucun codec sectoriel n'a été retesté.

Cette étape n'inclut pas encore la façade sectorielle `HfeConversionService` ni les conversions FAT12 restant dans l'inventaire.
