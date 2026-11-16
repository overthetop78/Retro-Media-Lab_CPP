# Contenu séquentiel exploré dans MFS

Sources C# consultées en lecture seule le 16 novembre 2026 : Exploration/Sequential/MediaSequentialDecodedBlock.cs, SequentialContentVolumeDetector.cs, SequentialContentFileSystemReader.cs et Interfaces/Exploration/Sequential/IMediaSequentialContent.cs dans F:/GW GUI/src/RetroMediaLab.MediaFileSystems. SHA-256 des quatre sources vérifiés contre l’inventaire. SequentialContentDecoderAdapter et les contrats/registre de décodeurs moteur ont aussi été examinés pour préserver la frontière ; ils restent à porter.

MFS reçoit uniquement des blocs déjà décodés et n’interprète aucun protocole machine. SequentialDecodedBlock possède octets et métadonnées, expose des vues constantes et valide la position. SequentialContent fournit décodeur optionnel, blocs et diagnostics, avec messages à clés ajoutés au contrat C++ existant de traduction. Les chaînes de diagnostic externes sont conservées comme sourceDiagnostics ; les messages structurés comme sourceMessages, sans dépendance d’App ou du moteur.

Le reader conserve l’ordre des blocs. Un nom absent/blanc interrompt le fichier et compte comme anonyme ; aucun nom n’est inventé. Les noms sont trimés pour le regroupement mais les métadonnées originales sont conservées. Seuls un nom identique ordinal et des numéros consécutifs valides réunissent deux blocs. Les numéros utilisent NumberStyles.None : chiffres seuls, int32 positif ou zéro ; espaces, signes et dépassements ne sont pas acceptés. L’addition du numéro précédent utilise int64 pour éviter le débordement.

Les données sont concaténées sans transformation. L’intégrité du fichier vaut false dès qu’un bloc est invalide, true si tous sont valides, sinon inconnue. Le type natif et les métadonnées viennent du premier bloc. La référence de stockage est plafonnée à INT32_MAX comme dans le C#. Les fichiers trop volumineux sont refusés avant dépassement ; le résultat local et ses buffers sont libérés si une exception/annulation survient. Les fichiers sans nom ne deviennent pas des entrées artificielles.

Le volume garde son nom fourni, sa capacité, un espace libre inconnu, les attributs format/décodeur, diagnostics dédupliqués et avertissements structurés pour blocs anonymes, intégrité et absence de fichier nommé.

Le détecteur existait déjà dans Exploration/Volumes/Detectors.cpp et la composition par défaut : il exige Tape, accès séquentiel et absence d’accès aléatoire. La priorité nullable longueur de représentation, longueur connue, taille du fichier est conservée ; une longueur explicitement nulle ne déclenche pas de repli vers une longueur connue. Ce code n’est pas dupliqué ; son état d’inventaire est corrigé après contrôle.

Le registre des décodeurs, l’adaptateur moteur vers ce contrat MFS et les décodeurs de protocoles restent à porter avant une exploration de bande de bout en bout depuis App. Aucun raccordement fictif n’est déclaré complet.
