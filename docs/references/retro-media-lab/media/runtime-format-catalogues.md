# Catalogues dynamiques et capacités Greaseweazle

Références C# : RuntimeImageFormatCatalog.cs, CapabilityAwareImageFormatCatalog.cs, GwFormatCapabilities.cs, GwFormatCapabilitiesParser.cs et GwFormatArgument.cs. Les quatre identifiants spéciaux de BuiltInDiskDefinitions.Supports sont repris pour l'adaptation des capacités physiques ; son ajout d'argument et son fichier externe ne sont pas portés dans ce bloc.

mergeRuntimeImageFormats conserve l'ordre et la présentation des formats existants. Pour une entrée de même identifiant, seules les extensions et les compatibilités source sont remplacées. Un nouveau format est ajouté ; les mises à jour suivantes de ce même identifiant conservent sa première présentation.

GwFormatCapabilities représente les formats logiciels annoncés par un texte d'aide fourni. Aucun appel matériel, lancement de commande ou accès à App n'est ajouté. Une liste d'identifiants ou d'extensions non vide rend les capacités connues, comme le C#. parseGwReadHelp limite l'analyse aux deux sections FORMAT options/Supported file suffixes, avec les mêmes expressions de reconnaissance, déduplication et casse insensible.

Si les capacités sont inconnues, le catalogue conserve ses drapeaux. Sinon, un format déjà désactivé pour les deux opérations reste désactivé. Les autres conservent leurs possibilités initiales uniquement s'ils sont supportés après alias, s'ils sont raw.scp/raw.hfe ou s'ils font partie des quatre définitions internes particulières. Cela ne constitue pas une mesure des capacités d'un lecteur physique.

Les nouveaux formats sont ajoutés dans l'ordre insensible à la casse. IMG est préféré, puis SCP, puis la première extension ; les définitions entièrement bitcell proposent uniquement SCP et HFE disponibles, dans cet ordre. Famille, détail, noms de suffixes et tag sont dérivés des identifiants techniques avec le même vocabulaire connu que le C#.

Les formats existants gardent leurs clés de traduction. Les noms découverts sont des invariants techniques calculés à l'exécution, exposés séparément des clés. Les fonctions de résolution utilisent le résolveur de langue courant pour les clés et rendent les invariants directement ; en l'absence de résolveur, une clé reste une clé. Aucune phrase utilisateur nouvelle n'est codée en dur.

La résolution des dimensions partage maintenant gwFormatArgument, évitant deux implémentations divergentes des alias. Toutes les tables et valeurs techniques se trouvent dans Constants.h. Les résultats sont possédés par valeur ; aucune référence à une liste temporaire n'est conservée.
