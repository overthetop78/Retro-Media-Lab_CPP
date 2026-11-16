# Registre et sélection SCP

Référence : `Images/Formats/Floppy/Scp/Decoding/ScpSectorDecodingComposition.cs`, `Decoding/Sectors/ScpCandidateRegistry.cs`, `ScpSectorImageReader.cs`, `ScpCandidateIds.cs`, `ScpFormatSelection.cs`, `ScpCandidateFailure.cs` du dépôt C# consulté en lecture seule.

La composition partage le lecteur/caches SCP de `RuntimeDetails::Services`. Les callbacks possèdent les reconstructeurs et le registre de décodage. Les sélections explicites, les candidats par défaut et l'ordre des familles reprennent le C#. DEC reste dans la sélection par famille et hors des candidats par défaut. Les définitions sont consultées avant les préfixes généraux. Les lectures automatiques Commodore par chemin ignorent le format, comme le callback progressif C# ; les lectures par flux lui transmettent le format.

Le premier décodage est conservé ; le premier système de fichiers reconnu gagne. Le registre MediaFileSystems existant effectue cette reconnaissance. Les rejets de format permettent de continuer ; les erreurs d'accès et annulations remontent. L'échec global réutilise `ReaderAllRejected`, déjà traduit dans les 29 langues, et conserve les exceptions originales et les identifiants des candidats. Pour les flux, ces causes sont également conservées (le C# les supprimait). Un candidat sans fonction flux est ignoré en automatique et refusé explicitement avec le diagnostic existant `UnsupportedRawFormat`.

Validation : compilation Debug MediaEngine réussie ; vérification ciblée des inscriptions, ordre des familles, sélection explicite sans repli, premier décodage, priorité à la reconnaissance, filtrage flux, causes originales, erreur IO et annulation réussie. Audit des littéraux : 1468 fichiers C++, 29 langues, réussi. Aucun codec n'a été revalidé.

La composition est disponible dans les services ; le raccordement aux convertisseurs et à l'inspection reste à porter. Les noms affichés des candidats restent à raccorder avec l'inspection. La lecture du conteneur SCP continue à retourner le flux brut.
