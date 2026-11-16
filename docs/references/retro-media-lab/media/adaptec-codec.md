# Codec physique Adaptec

Références C# en lecture seule : les sept sources `Images/Formats/HardDisk/Adaptec/`, avec empreintes vérifiées dans l'inventaire. Les paramètres initiaux ont été portés avec le DSC ; cette tâche complète les constantes, contrôle d'erreur, géométrie, champs physiques, fenêtres d'écriture et lecteur logique.

Le codec réutilise les révolutions en nanosecondes, reconstruction de transitions et secteurs existants. Il sépare encodage, décodage, contrôle d'erreur, fenêtre DATA, géométrie et accès logique. Aucun accès matériel ni fichier n'est ajouté au codec. Le contrôle au polynôme 0x41044185 est celui du format média : aucune correction matérielle ni relecture du contrôleur n'est simulée.

L'encodage respecte les cellules de 100 ns, marques à horloge manquante, préambules et gaps du C#. L'ordre physique des secteurs est fourni par l'appelant. Un secteur indisponible avec taille connue produit uniquement un gap, sans identité ni données fictives. Les paramètres occupent les premiers drapeaux, les contrôles DATA enregistrés sont conservés et les états d'intégrité demandés sont représentés.

Le décodage reconstruit les bits sans adaptation d'horloge. Les marques sont qualifiées par le préambule NRZ nul, puis les champs sont lus circulairement. La recherche DATA s'arrête au prochain ID et respecte les marques bloquantes. Sans taille connue, seul un champ DATA au contrôle valide détermine sa taille ; avec taille connue, le champ endommagé et ses octets de contrôle restent exposés. Les identités défectueuses ou au contrôle invalide ne deviennent pas des adresses utilisables.

La fenêtre d'écriture appartient à son appelant : elle contient les transitions DATA et le premier clock de raccordement, en conservant une éventuelle marque A1 à horloge manquante. Une identité non unique, défectueuse, absente ou invalide ne permet pas cette écriture. Le codec ne modifie pas la révolution source.

La géométrie est relue depuis les neuf drapeaux de paramètres autour de l'adresse zéro, après validation de toutes les identités de la piste. Le lecteur logique est synchronisé, découvre sa géométrie sur la première piste et sa capacité logique depuis le dernier drapeau de fin. Il conserve un seul décodage de piste. Un secteur déplacé est recherché à partir de sa piste logique et dans les pistes suivantes, comme en C# ; une identité manquante ou dupliquée échoue, sans inventer des zéros.

L'adaptateur de blocs possède un shared_ptr vers le lecteur. Il résout sa capacité lors de la création pour que `RandomAccessData::length() noexcept` puisse rester sans exception ; les données sont ensuite lues à la demande. Les callbacks doivent conserver les sources empruntées nécessaires à leur propre durée de vie. Les verrous et buffers sont libérés par RAII sur tous les chemins.

Le conteneur MfmEmu et le raccordement global des services restent à porter. Cette tâche ne constitue pas une validation d'émulation du contrôleur Adaptec.
