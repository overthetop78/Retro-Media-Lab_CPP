# Pistes physiques du runtime secteur/bloc

Références C# en lecture seule : MediaFluxRevolutionData, MediaFluxTrackData, MediaFluxTrackDataFunctions, MediaRuntimeDisk.Tracks et MediaRuntimeDisk.WinchesterTracks. Les flux publics sont possédés par valeur et consultables en lecture seule ; leurs durées et intervalles sont en nanosecondes. Les encodeurs/décodeurs et les modèles de représentation existants restent les seuls algorithmes de format.

## Flux neutre et fenêtre

Le format binaire conserve la signature little-endian 0x31465747, le nombre de révolutions, puis les périodes et intervalles uint32. Les mêmes bornes C# sont appliquées : 255 révolutions, 16 777 216 intervalles par révolution, longueur sérialisée maximale 2 147 483 647. Révolution de période nulle, intervalle nul, donnée tronquée, signature incorrecte et données finales superflues sont refusés. Une fin silencieuse de révolution ne crée aucune impulsion.

Le remplacement de fenêtre garde seulement les transitions hors de la porte d’écriture. Il valide l’ordre strict des impulsions émises, gère la fermeture exclusive/inclusive et le passage à l’index, et supprime une impulsion lorsqu’un passage ultérieur de la porte la réécrit sans impulsion de remplacement. L’entrée n’est pas modifiée. Les limites et unités sont dans un en-tête de constantes nommé, enregistré explicitement dans l’audit existant sans changement des contrôles.

## Projection vers une image

La lecture physique encode l’image sectorielle avec les politiques existantes. ImageDisk reprend le mode FM/MFM et le débit de ses descripteurs. Une écriture de fenêtre conserve les impulsions réelles et décode les champs observés ; le remplacement sectoriel utilise les anciens modèles de piste pour préserver la géométrie. Lorsque le conteneur ne peut pas publier les champs, le flux et la projection en mémoire restent accessibles, mais flush réessaie la projection puis refuse la publication destructive. Une écriture physique suivante peut réparer la piste.

Pour Acorn DAT/DSC, la lecture encode les champs Adaptec avec l’entrelacement ou le mode séquentiel et les paramètres physiques du descripteur. Les plages indisponibles produisent une absence de champ de données. La projection vérifie toute la piste avant de modifier les blocs : nombre et unicité des adresses, taille, intégrité des en-têtes/données, absence de défaut d’adresse, paramètres et marque du dernier secteur. Une piste physique en attente interdit l’écriture sectorielle si sa projection reste invalide. La lecture sectorielle d’une piste déjà écrite consulte son flux réel. Le changement de géométrie explicite invalide les caches de piste et la fermeture les libère, même si la publication échoue.

## Vérifications

Compilation Debug de MediaEngine réussie. Un avertissement de masquage de membre dans le formatage a été corrigé ; la recompilation ciblée passe sans cet avertissement. Le banc vérifie les octets exacts du format binaire, les erreurs de signature, compte, intervalle nul, troncature et octets superflus. Les fenêtres ordinaires, inclusives, traversant l’index et couvrant plusieurs tours sont vérifiées sur des résultats attendus explicites.

Une IBM 720 fournit neuf secteurs MFM. Après effacement physique d’une piste, flush refuse la publication, conserve les changements et laisse le fichier initial intact. La réécriture de ses impulsions initiales permet de sauvegarder puis de relire les neuf secteurs, avec fichier identique. Une piste ImageDisk FM 250 contenant deux secteurs passe par la lecture physique, la réécriture intégrale, la sauvegarde et la lecture sectorielle.

Une DAT/DSC de 33 secteurs est encodée et décodée avec ses paramètres physiques. Une fenêtre Adaptec modifie un secteur, vérifié dans le flux puis après sauvegarde et réouverture. Un effacement physique provoque un refus de publication et d’écriture sectorielle ; le fichier reste intact. La restauration de la piste permet la sauvegarde suivante. Une fermeture avec piste toujours invalide signale l’erreur et remet la mécanique possédée dans son état initial.

Trois diagnostics générés avec Argos dans 29 langues, relus et corrigés notamment pour la négation coréenne, le texte arabe resté anglais, les termes hébreux et le chinois traditionnel. Les 30 DLL de catalogues compilent. Audit des constantes : 1 418 fichiers C++, 29 langues, aucune valeur applicative hors constantes nommées. Le banc et toutes ses images/exécutables temporaires sont supprimés après utilisation.

## Suite

Les backends runtime des conteneurs conservant directement le flux ou les cellules MFM restent à porter, ainsi que leur sélection dans la façade de préparation. Le présent bloc achève les méthodes de piste du runtime des images déjà décodées en secteurs ou blocs ; il ne certifie pas le portage global MediaEngine/MediaFileSystems/MediaAnalysis.
